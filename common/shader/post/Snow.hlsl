//
//  Snow.hlsl
//  Square
//
//  The falling snow (see PostEffectSnow): the view ray of the pixel walks the cells of a grid in
//  the world (a 3D DDA), the grid falling with the time and drifting with the wind; a cell may
//  hold a flake (a random place, size): a short segment along its fall (the motion blur), whose
//  distance from the ray gives its coverage. Out of focus near the camera, at least a pixel wide
//  (the coverage spread on it), fading with the distance, hidden behind the geometry of the
//  pixel (G-Buffer positions), lit by the frame behind it.
//
#include <Camera>
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_source);
Sampler2D(g_position);
Mat4  snow_inverse; //inverse of the view projection
Vec2  snow_size;    //pixels of the frame
float snow_time;    //seconds
Vec4  snow_params;  //density, spacing (side of a cell), near distance, max distance
Vec4  snow_motion;  //fall speed, radius of a flake, wind x, wind z (world units)
Vec4  snow_color;   //rgb, intensity
Vec4  snow_extra;   //G-Buffer (1: yes), angle of a pixel, shutter (seconds), focus distance

//4 random values [0, 1) of a cell (its coordinates wrapped: small numbers for the sin, the
//grid repeats every 256 cells)
Vec4 snow_hash(in Vec3 cell)
{
	cell = cell - 256.0 * floor(cell / 256.0);
	Vec4 p = Vec4(dot(cell, Vec3(127.1, 311.7, 74.7)), dot(cell, Vec3(269.5, 183.3, 246.1)),
	              dot(cell, Vec3(419.2, 371.9, 113.3)), dot(cell, Vec3(113.5, 271.9, 337.1)));
	return frac(sin(p) * 43758.5453);
}

//distance between a ray (origin, direction) and a segment (center, half along y)
float snow_distance(in Vec3 origin, in Vec3 direction, in Vec3 center, in float half_length, out float along)
{
	//the point of the segment nearest to the ray: the lines' closest points, clamped on the segment
	Vec3  w = origin - center;
	float b = direction.y;               //dot(direction, up)
	float d = dot(direction, w);
	float e = w.y;                       //dot(up, w)
	float denominator = 1.0 - b * b;
	float s = denominator > 0.0001 ? clamp((e - b * d) / denominator, -half_length, half_length) : 0.0;
	Vec3  nearest = center + Vec3(0.0, s, 0.0);
	along = dot(nearest - origin, direction);
	Vec3  closest = origin + direction * max(along, 0.0);
	return length(nearest - closest);
}

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / snow_size;
	Vec3 color = texture2DLod(g_source, uv, 0.0).rgb;
	//the distance of the geometry of the pixel (none: far away)
	float scene = 1e6;
	if (snow_extra.x > 0.5)
	{
		Vec4 g_pos = texture2DLod(g_position, uv, 0.0);
		if (g_pos.w > 0.5) scene = length(g_pos.xyz - camera.m_position);
	}
	//the view ray of the pixel
#ifdef GLSL_BACKEND
	Vec2 ndc = Vec2(uv.x * 2.0 - 1.0, uv.y * 2.0 - 1.0);
#else
	Vec2 ndc = Vec2(uv.x * 2.0 - 1.0, 1.0 - uv.y * 2.0);
#endif
	Vec4 far_point = mul(Vec4(ndc, 0.5, 1.0), snow_inverse);
	Vec3 direction = normalize(far_point.xyz / far_point.w - camera.m_position);
	//the grid falls and drifts: the ray in the space of the grid goes the other way
	Vec3 origin = camera.m_position + Vec3(-snow_motion.z, snow_motion.x, -snow_motion.w) * snow_time;
	float spacing = snow_params.y;
	float limit = min(snow_params.w, scene);
	//3D DDA: the cells along the ray
	Vec3 safe = Vec3(abs(direction.x) > 1e-5 ? direction.x : 1e-5, abs(direction.y) > 1e-5 ? direction.y : 1e-5, abs(direction.z) > 1e-5 ? direction.z : 1e-5);
	Vec3 cell = floor(origin / spacing);
	Vec3 stride = sign(safe);
	Vec3 delta = abs(spacing / safe);
	Vec3 next = ((cell + max(stride, Vec3(0.0, 0.0, 0.0))) * spacing - origin) / safe;
	float travelled = 0.0;
	float coverage = 0.0;
	float half_length = snow_motion.x * snow_extra.z * 0.5;
	for (int i = 0; i < 48; ++i)
	{
		if (travelled > limit) break;
		Vec4 random = snow_hash(cell);
		if (random.w < snow_params.x)
		{
			//the flake of the cell: a place inside it (it stays in), a size
			Vec3  center = (cell + 0.5 + (random.xyz - 0.5) * 0.6) * spacing;
			float along = 0.0;
			float d = snow_distance(origin, direction, center, half_length, along);
			if (along > snow_params.z && along < scene)
			{
				float radius = snow_motion.y * (0.6 + 0.8 * random.x);
				//at least a pixel wide: the coverage spread on it (no shimmer far away)
				float pixel = along * snow_extra.y;
				float width = max(radius, pixel);
				float spread = (radius * radius) / (width * width);
				//out of focus near: a soft disc, wider, fainter
				float blur = saturate(1.0 - along / snow_extra.w);
				float soft = width * (1.0 + 2.0 * blur);
				float flake = (1.0 - smoothstep(soft * lerp(0.4, 0.0, blur), soft, d)) * spread / (1.0 + 4.0 * blur * blur);
				//fading with the distance
				flake *= 1.0 - smoothstep(snow_params.w * 0.5, snow_params.w, along);
				coverage = 1.0 - (1.0 - coverage) * (1.0 - saturate(flake));
			}
		}
		//to the next cell
		if (next.x < next.y && next.x < next.z)
		{
			travelled = next.x; next.x += delta.x; cell.x += stride.x;
		}
		else if (next.y < next.z)
		{
			travelled = next.y; next.y += delta.y; cell.y += stride.y;
		}
		else
		{
			travelled = next.z; next.z += delta.z; cell.z += stride.z;
		}
	}
	//lit by the scene: the light of the frame behind the flake
	float light = 0.55 + 0.6 * saturate(dot(color, Vec3(0.299, 0.587, 0.114)));
	return Vec4(lerp(color, snow_color.rgb * light, coverage * snow_color.a), 1.0);
}
