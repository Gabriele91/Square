////////////////
#pragma once
#include <Camera>
#include <Transform>
#include <Vertex>
#include <Support>
#include <Matrix>
// NOTE: GammaCorrection must be included BEFORE SurfacePBR: the include
// preprocessor expands #include inside dead #if branches too, and marks the
// files as already-included (pragma once); including it afterwards would leave
// its code only inside the discarded rendering branch.
#include <GammaCorrection>
#include <SurfacePBR>
////////////////
struct VertexShaderOutput
{
	Vec4 m_position       : SV_POSITION;  // vertex position (system value)
	Vec2 m_uv             : TEXCOORD0;    // interpolated uv map
	Vec4 m_world_position : TEXCOORD1;    // vertex position (wolrd space)
	//TBN
	Vec3 m_normal   : TEXCOORD3;    // normal
	Vec3 m_tangent  : TEXCOORD4;    // normal
	Vec3 m_binomial : TEXCOORD5;    // normal
};

//texture
Sampler2D(albedo_map);
Sampler2D(emmisive_map);
Sampler2D(metallic_map);
Sampler2D(roughness_map);
Sampler2D(occlusion_map);
Sampler2D(normal_map);
//global uniform
Vec4  color;
float metallic;
float roughness;
Vec3 emmisive;
float mask;
float dither;
#ifdef SURFACE_SNOW_COVER
// Snow cover (PBRSnowCover): snow laid on what faces up (the world normal), its edge broken by
// the brightness of the surface under it (the snow in the hollows and on the ledges first, the
// cracks out), the bumps of the surface smoothed under it; the snow a texture tiled from above
Sampler2D(snow_cover_map);
Vec4 snow_cover; // normal y where it starts, softness of its edge, 1 / size of its texture (world), its roughness
#endif
#ifdef SURFACE_TRAILS
// Trails (PBRSnow): a map over the ground (from above, world x/z) of how deep it is pressed
// (R, [0, 1]: a groove), drawn by the game: the pressed snow darker, smoother, its normal
// following the sides of the groove
Sampler2D(trail_map);
Vec4 trail_area;  // x, z of its corner (world); 1 / its size along x, z
Vec4 trail_style; // darkening, slope of the sides (normal), roughness kept, depth (world units)

// the albedo alpha of a surface with trails is where they can be (1: the snow; ice, rock: 0)

// the depth of the trails at a world point x/z, [0, 1]
float trail_depth(Vec2 world_xz)
{
	return texture2DLod(trail_map, (world_xz - trail_area.xy) * trail_area.zw, 0.0).r;
}

// parallax into the grooves: from the surface the view ray goes down (layers of the depth of a
// groove) until it is under the pressed snow; the point (x/z) seen there. On the flat snow (no
// depth) it stops at once
Vec2 trail_parallax(Vec3 world)
{
	const int steps = 14;
	Vec3  view = normalize(world - camera.m_position);
	// x/z of the ray per layer (grazing views clamped: no long smears)
	Vec2  step_xz = view.xz / max(-view.y, 0.2) * (trail_style.w / float(steps));
	float layer = 0.0;
	Vec2  p = world.xz;
	float depth = trail_depth(p);
	Vec2  previous_p = p;
	float previous_gap = depth;
	for (int i = 0; i < steps; ++i)
	{
		if (layer >= depth) break;
		previous_p = p;
		previous_gap = depth - layer;
		p += step_xz;
		layer += 1.0 / float(steps);
		depth = trail_depth(p);
	}
	// between the last two layers: where the ray met the snow
	float gap = layer - depth;
	float t = previous_gap + gap > 0.0001 ? previous_gap / (previous_gap + gap) : 1.0;
	return lerp(previous_p, p, saturate(t));
}
#endif

#ifdef SURFACE_WATER
// Water (PBRWater): its texture flowing (a waterfall: water_flow.xy), two layers of ripples of
// the normal map moving across each other, a Fresnel: clear where looked at from above, the sky
// reflected where looked at grazing (a gradient: its horizon, its top), the lights (the glint
// of the sun) as on any surface. water_time.x: the seconds of the game (set every frame)
Vec4 water_time;  // x: seconds
Vec4 water_flow;  // xy: the speed of its uv (per second), z: the scale of the ripples (uv), w: their strength
Vec4 water_style; // x: alpha looking down, y: alpha grazing, z: the reflection, w: the Fresnel power
Vec4 water_sky;   // rgb: the sky at the horizon (linear)
Vec4 water_top;   // rgb: the sky at the top (linear)
// a waterfall (water_fall.x 1): its uv v from its top (0) to its foot (1), u across it (0 to 1);
// its albedo map a noise (streaks along v) scrolled down in two layers (water_flow.xy, the
// second faster): where they are bright the light water (color), else the deep one
// (water_deep), sharp bands (the threshold); foam at its sides and at its foot, its sides
// dissolving
Vec4 water_fall;  // x: 1 a waterfall, y: the threshold of the bands, z: the foam of the sides (u), w: the foam of the foot (v)
Vec4 water_deep;  // rgb: the deep water (linear), w: how many times the noise repeats down it
// the wakes of the hovercraft (drawn by the game, from above: world x/z, R how fresh): rings of
// waves along them, moving out, foam where they are fresh
Sampler2D(wake_map);
Vec4 wake_area;   // x, z of its corner (world); 1 / its size along x, z
Vec4 wake_style;  // x: the strength of its waves, y: their rings (per unit of the map), z: their speed, w: its foam
float wake_height(Vec2 world_xz, float t)
{
	float w = texture2DLod(wake_map, (world_xz - wake_area.xy) * wake_area.zw, 0.0).r;
	return w * sin(w * wake_style.y - t * wake_style.z);
}
#endif

// Dithered opacity: 4x4 ordered (Bayer) threshold in (0,1) of a screen pixel.
// A pixel is kept when its alpha is above the threshold, so the share of kept
// pixels follows the alpha and the surface stays opaque (deferred, shadows...).
float dither_threshold(Vec2 pixel)
{
	static const float bayer[16] =
	{
		 0.0,  8.0,  2.0, 10.0,
		12.0,  4.0, 14.0,  6.0,
		 3.0, 11.0,  1.0,  9.0,
		15.0,  7.0, 13.0,  5.0
	};
	uint2 p = uint2(pixel) & 3;
	return (bayer[p.y * 4 + p.x] + 0.5) / 16.0;
}

VertexShaderOutput vertex(Position3DNormalTangetBinomialUV input)
{
	VertexShaderOutput output;
	output.m_world_position = mul_model(input.m_position);
	output.m_position = mul_view_projection(output.m_world_position.xyz);
	output.m_uv = input.m_uv;

	Mat3 normal3x3    = (Mat3)transform.m_inv_model;
	output.m_normal   = mul(normal3x3, input.m_normal);
	output.m_tangent  = mul(normal3x3, input.m_tangent);
	output.m_binomial = mul(normal3x3, input.m_binomial);

	return output;
}

surface(VertexShaderOutput input)
{
	// Note, in glTF 
    // RGB (metallic, norma maps)
    // SRGB (emmisive, albedo)
	SurfaceData data = DefaultSurfaceData();
	// World position
	data.m_position = input.m_world_position;
#ifdef SURFACE_WATER
	// the texture flows (the uv of the surface kept: the waterfall, its sides, its foot)
	Vec2 water_uv = input.m_uv;
	input.m_uv += water_flow.xy * water_time.x;
#endif
	// Diffuse/albedo
	Vec4 albedo_color = to_rgb_space(texture2D(albedo_map, input.m_uv));
	if (albedo_color.a <= mask) discard;
	data.m_albedo = albedo_color.rgb * color.rgb;
	// Alpha
	data.m_alpha = albedo_color.a * color.a;
	// Dithered opacity: drop the pixels of the pattern above the alpha
	if (dither > 0.5)
	{
		if (data.m_alpha <= dither_threshold(input.m_position.xy)) discard;
		data.m_alpha = 1.0;
	}
	// Emmisive
	data.m_emmisive = to_rgb_space(texture2D(emmisive_map, input.m_uv).rgb) * emmisive;
	// Normal
	Vec4 normal_color = texture2D(normal_map, input.m_uv);
	Mat3 TBN = Mat3(input.m_tangent, input.m_binomial, input.m_normal);
	data.m_normal = normalize(mul(normal_from_texture(normal_color), TBN));
	// AO
	data.m_occlusion = texture2D(occlusion_map, input.m_uv).r;
	// Metallic
	data.m_metallic = texture2D(metallic_map, input.m_uv).b * metallic; // glTF uses B channel for the metallic
	// Roughness
	data.m_roughness = texture2D(roughness_map, input.m_uv).g * roughness; // glTF uses G channel for the roughness
#ifdef SURFACE_SNOW_COVER
	{
		Vec3  snow_albedo = to_rgb_space(texture2D(snow_cover_map, input.m_world_position.xz * snow_cover.z)).rgb;
		float bright = dot(data.m_albedo, Vec3(0.333, 0.333, 0.333));
		float up = data.m_normal.y + (bright - 0.25) * 0.5;
		float cover = smoothstep(snow_cover.x - snow_cover.y, snow_cover.x + snow_cover.y, up);
		data.m_albedo = lerp(data.m_albedo, snow_albedo, cover);
		data.m_normal = normalize(lerp(data.m_normal, normalize(input.m_normal), cover * 0.7));
		data.m_roughness = lerp(data.m_roughness, snow_cover.w, cover);
		data.m_metallic *= 1.0 - cover;
	}
#endif
#ifdef SURFACE_WATER
	{
		// the ripples: two layers of the normal map, moving across each other
		float t = water_time.x;
		Vec2  uv = input.m_uv * water_flow.z;
		Vec3  n1 = normal_from_texture(texture2D(normal_map, uv + Vec2(0.11, 0.07) * t));
		Vec3  n2 = normal_from_texture(texture2D(normal_map, uv * 1.73 + Vec2(-0.08, 0.12) * t));
		Vec3  ripple = normalize(Vec3((n1.xy + n2.xy) * water_flow.w, 1.0));
		data.m_normal = normalize(mul(ripple, TBN));
		// Fresnel: clear from above, the sky reflected grazing
		Vec3  view = normalize(camera.m_position - input.m_world_position.xyz);
		float facing = saturate(dot(data.m_normal, view));
		float fresnel = 0.02 + 0.98 * pow(1.0 - facing, water_style.w);
		Vec3  reflected = reflect(-view, data.m_normal);
		Vec3  sky = lerp(water_sky.rgb, water_top.rgb, saturate(reflected.y));
		// the wakes: the slope of their waves bends the normal, foam where they are fresh
		{
			Vec2  xz = input.m_world_position.xz;
			float e = 0.25;
			float dx = wake_height(xz + Vec2(e, 0.0), t) - wake_height(xz - Vec2(e, 0.0), t);
			float dz = wake_height(xz + Vec2(0.0, e), t) - wake_height(xz - Vec2(0.0, e), t);
			data.m_normal = normalize(data.m_normal - Vec3(dx, 0.0, dz) * wake_style.x);
			float fresh = texture2DLod(wake_map, (xz - wake_area.xy) * wake_area.zw, 0.0).r;
			float foam = smoothstep(0.55, 1.0, fresh) * wake_style.w;
			data.m_albedo = lerp(data.m_albedo, Vec3(0.9, 0.95, 0.95), foam);
			data.m_alpha = lerp(data.m_alpha, 1.0, foam);
			facing = saturate(dot(data.m_normal, view));
			fresnel = 0.02 + 0.98 * pow(1.0 - facing, water_style.w);
			reflected = reflect(-view, data.m_normal);
			sky = lerp(water_sky.rgb, water_top.rgb, saturate(reflected.y));
		}
		data.m_alpha *= lerp(water_style.x, water_style.y, fresnel);
		data.m_emmisive += sky * fresnel * water_style.z;
		data.m_albedo *= 1.0 - fresnel;
		if (water_fall.x > 0.5)
		{
			// the waterfall: two layers of the noise falling (the second faster, shifted)
			Vec2  fall = Vec2(water_uv.x, water_uv.y * water_deep.w);
			float a = to_rgb_space(texture2D(albedo_map, fall + water_flow.xy * t)).r;
			float b = to_rgb_space(texture2D(albedo_map, fall * Vec2(1.37, 0.8) + water_flow.xy * t * 1.6 + Vec2(0.37, 0.11))).r;
			float noise = (a + b) * 0.5;
			float band = smoothstep(water_fall.y - 0.04, water_fall.y + 0.04, noise);
			float across = min(water_uv.x, 1.0 - water_uv.x);
			float side = 1.0 - smoothstep(0.0, water_fall.z, across);
			float foot = smoothstep(1.0 - water_fall.w, 1.0, water_uv.y);
			float foam = saturate(band + side * smoothstep(0.25, 0.55, noise) + foot * smoothstep(0.15, 0.45, noise));
			data.m_albedo = lerp(water_deep.rgb, color.rgb, foam);
			data.m_emmisive += data.m_albedo * 0.35;
			// opaque where it foams, its sides dissolving into the noise
			data.m_alpha = lerp(0.72, 1.0, foam) * color.a * smoothstep(0.0, water_fall.z * 0.6, across + (noise - 0.5) * water_fall.z * 0.8);
			data.m_roughness = lerp(0.05, 0.5, foam);
		}
	}
#endif
#ifdef SURFACE_TRAILS
	// Trails: only on the snow (the albedo alpha: not on the ice, the rock), not a transparency
	float trail_snow = albedo_color.a;
	data.m_alpha = color.a;
	// the point of the groove seen (parallax), its depth, its slope from the texels around
	Vec2  trail_seen = trail_snow > 0.0 ? trail_parallax(input.m_world_position.xyz) : input.m_world_position.xz;
	Vec2  trail_uv = (trail_seen - trail_area.xy) * trail_area.zw;
	Vec2  trail_texel = 1.0 / textureSize2D(trail_map, 0);
	float trail = texture2DLod(trail_map, trail_uv, 0.0).r * trail_snow;
	float trail_dx = texture2DLod(trail_map, trail_uv + Vec2(trail_texel.x, 0.0), 0.0).r - texture2DLod(trail_map, trail_uv - Vec2(trail_texel.x, 0.0), 0.0).r;
	float trail_dz = texture2DLod(trail_map, trail_uv + Vec2(0.0, trail_texel.y), 0.0).r - texture2DLod(trail_map, trail_uv - Vec2(0.0, trail_texel.y), 0.0).r;
	// the ground lower where it is deeper: the normal leans toward the deeper side
	data.m_normal = normalize(data.m_normal + Vec3(trail_dx, 0.0, trail_dz) * (trail_style.y * trail_snow));
	data.m_albedo *= 1.0 - trail * trail_style.x;
	data.m_roughness = lerp(data.m_roughness, data.m_roughness * trail_style.z, trail);
#endif
	//return
	surface_return(data);
}

