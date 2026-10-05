//
//  DOF.hlsl
//  Square
//
//  The depth of field (see PostEffectDOF): the circle of confusion of a pixel from the distance
//  of its world position (G-Buffer) to the camera, a gather of a disc of samples around it (a
//  golden angle spiral), each sample weighted by whether its own circle reaches the pixel (a
//  sharp thing in front does not smear over the blur behind it).
//
#include <Camera>
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_source);
Sampler2D(g_position);
Vec2 dof_size;    //pixels of the frame
Vec4 dof_focus;   //start, end of the sharp range; the near range, the far range (world units)
Vec4 dof_params;  //max radius (pixels), samples, near on
Vec4 dof_motion;  //motion blur: pixels at its most, from, to (screen x shares)

//the circle of confusion of a world position (background: the farthest), [0, 1]
float dof_coc(in Vec4 g_pos)
{
	if (g_pos.w < 0.5) return 1.0;
	float distance = length(g_pos.xyz - camera.m_position);
	float far = saturate((distance - dof_focus.y) / dof_focus.w);
	float near = saturate((dof_focus.x - distance) / dof_focus.z) * dof_params.z;
	return max(far, near);
}

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2  uv = input.m_position.xy / dof_size;
	Vec2  texel = 1.0 / dof_size;
	float coc = dof_coc(texture2DLod(g_position, uv, 0.0));
	Vec3  center = texture2DLod(g_source, uv, 0.0).rgb;
	float radius = coc * dof_params.x;
	//the motion: along x, on what is near (in focus or nearer), growing to the right
	Vec4  g_center = texture2DLod(g_position, uv, 0.0);
	float near_focus = g_center.w > 0.5 && length(g_center.xyz - camera.m_position) < dof_focus.y + 4.0 ? 1.0 : 0.0;
	float motion = dof_motion.x * near_focus * smoothstep(dof_motion.y, dof_motion.z, uv.x);
	if (motion >= 1.0)
	{
		Vec3  msum = Vec3(0.0, 0.0, 0.0);
		float mweight = 0.0;
		for (int m = 0; m < 16; ++m)
		{
			float s = (float(m) / 15.0 - 0.5) * motion;
			Vec2  suv = uv + Vec2(s * texel.x, 0.0);
			Vec4  sg = texture2DLod(g_position, suv, 0.0);
			//only the near things smeared (not the far background into them)
			float w = sg.w > 0.5 && length(sg.xyz - camera.m_position) < dof_focus.y + 4.0 ? 1.0 : 0.15;
			msum += texture2DLod(g_source, suv, 0.0).rgb * w;
			mweight += w;
		}
		center = msum / mweight;
	}
	if (radius < 0.5) return Vec4(center, 1.0);
	int   samples = int(dof_params.y);
	Vec3  sum = center;
	float weight = 1.0;
	for (int i = 1; i < 64; ++i)
	{
		if (i >= samples) break;
		//the golden angle spiral: even over the disc
		float r = sqrt(float(i) / float(samples));
		float a = float(i) * 2.39996323;
		Vec2  offset = Vec2(cos(a), sin(a)) * r * radius;
		Vec2  suv = uv + offset * texel;
		float scoc = dof_coc(texture2DLod(g_position, suv, 0.0)) * dof_params.x;
		//a sample counts as far as its own blur reaches the pixel (a sharp one in front: little)
		float w = saturate(scoc - r * radius + 1.0);
		sum += texture2DLod(g_source, suv, 0.0).rgb * w;
		weight += w;
	}
	return Vec4(sum / weight, 1.0);
}
