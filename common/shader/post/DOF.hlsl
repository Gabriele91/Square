//
//  DOF.hlsl
//  Square
//
//  The depth of field (see PostEffectDOF): the circle of confusion of a pixel from the distance
//  of its world position (G-Buffer) to the camera, a gather of a disc of samples around it (a
//  golden angle spiral), each sample weighted by whether its own circle reaches the pixel (a
//  sharp thing in front does not smear over the blur behind it). At the size of the frame (the
//  result), or smaller (DOFComposite puts it over the frame).
//
#include <Camera>
#include <GBufferPosition>
#include <Vertex>
#include <DeferredFullscreen>
#include <DOFCommon>

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2  uv = input.m_position.xy / dof_size;
	Vec2  texel = 1.0 / dof_size;
	Vec4  g_center = gbuffer_world(texture2DLod(g_position, uv, 0.0), uv);
	float coc = dof_coc(g_center);
	Vec3  center = texture2DLod(g_source, uv, 0.0).rgb;
	float radius = coc * dof_params.x;
	//the motion: along x, on what is near (in focus or nearer), growing to the right
	float motion = dof_motion_pixels(g_center, uv);
	if (motion >= 1.0)
	{
		Vec3  msum = Vec3(0.0, 0.0, 0.0);
		float mweight = 0.0;
		for (int m = 0; m < 16; ++m)
		{
			float s = (float(m) / 15.0 - 0.5) * motion;
			Vec2  suv = uv + Vec2(s * texel.x, 0.0);
			Vec4  sg = gbuffer_world(texture2DLod(g_position, suv, 0.0), suv);
			//only the near things smeared (not the far background into them)
			float w = dof_near_focus(sg) > 0.5 ? 1.0 : 0.15;
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
		float scoc = dof_coc(gbuffer_world(texture2DLod(g_position, suv, 0.0), suv)) * dof_params.x;
		//a sample counts as far as its own blur reaches the pixel (a sharp one in front: little)
		float w = saturate(scoc - r * radius + 1.0);
		sum += texture2DLod(g_source, suv, 0.0).rgb * w;
		weight += w;
	}
	return Vec4(sum / weight, 1.0);
}
