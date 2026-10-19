//
//  DeferredPresent.hlsl
//  Square
//
//  Final present pass of the deferred pipeline: samples the HDR light
//  accumulation buffer and writes it to the screen (with sRGB encoding when the
//  hardware framebuffer does not do it for us). Used instead of a raw target
//  copy because not every backend supports blitting between different formats
//  (e.g. RGBA16F -> BGRA8 on Metal). A frame smaller than the screen (a scene drawn
//  at a share of it, Drawer::resolution_scale) stretched to it, bilinear (its texels
//  read at their centers: the frame has no filter).
//
#include <Vertex>
#include <GammaCorrection>
#include <DeferredFullscreen>

Sampler2D(g_light);
Vec4 g_viewport; //the screen (pixels: x, y, width, height)

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 size = textureSize2D(g_light, 0);
	Vec2 screen = max(g_viewport.zw, Vec2(1.0, 1.0));
	//the texel under the pixel (as large: the texel itself), its four around it
	Vec2 texel = (input.m_position.xy - g_viewport.xy) / screen * size - 0.5;
	Vec2 base = floor(texel);
	Vec2 blend = texel - base;
	Vec2 uv = (base + 0.5) / size;
	Vec2 step = 1.0 / size;
	Vec4 color = lerp
	(
		  lerp(texture2DLod(g_light, uv, 0.0), texture2DLod(g_light, uv + Vec2(step.x, 0.0), 0.0), blend.x)
		, lerp(texture2DLod(g_light, uv + Vec2(0.0, step.y), 0.0), texture2DLod(g_light, uv + step, 0.0), blend.x)
		, blend.y
	);
	color.a = 1.0;
	// The light buffer is linear; encode to sRGB unless the target does it.
#if !defined(ENABLE_TARGET_SRGB)
	color = to_srgb_space(color);
#endif
	return color;
}
