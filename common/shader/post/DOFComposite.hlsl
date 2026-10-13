//
//  DOFComposite.hlsl
//  Square
//
//  The depth of field drawn smaller than the frame (see PostEffectDOF, DOF.hlsl) over the frame:
//  each pixel of the frame its circle of confusion and its motion (pixels of the frame); sharp,
//  the frame as it is; blurred, the small blur (bilinear), a blend of the two over the first
//  pixels of blur (no step where the blur starts).
//
#include <Camera>
#include <GBufferPosition>
#include <Vertex>
#include <DeferredFullscreen>
#include <DOFCommon>

Sampler2D(g_blur); //the depth of field, smaller

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2  uv = input.m_position.xy / dof_size;
	Vec4  g_center = gbuffer_world(texture2DLod(g_position, uv, 0.0), uv);
	Vec3  center = texture2DLod(g_source, uv, 0.0).rgb;
	float radius = dof_coc(g_center) * dof_params.x;
	float motion = dof_motion_pixels(g_center, uv);
	//how blurred: none under half a pixel of radius (a pixel of motion), all from two
	float blurred = saturate(max(radius - 0.5, motion - 1.0) / 1.5);
	Vec3  color = center;
	if (blurred > 0.0)
	{
		color = lerp(center, texture2DLod(g_blur, uv, 0.0).rgb, blurred);
	}
	return Vec4(color, 1.0);
}
