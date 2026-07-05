//
//  DeferredPresent.hlsl
//  Square
//
//  Final present pass of the deferred pipeline: samples the HDR light
//  accumulation buffer and writes it to the screen (with sRGB encoding when the
//  hardware framebuffer does not do it for us). Used instead of a raw target
//  copy because not every backend supports blitting between different formats
//  (e.g. RGBA16F -> BGRA8 on Metal).
//
#include <Vertex>
#include <GammaCorrection>
#include <DeferredFullscreen>

Sampler2D(g_light);

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 size = textureSize2D(g_light, 0);
	Vec2 uv = input.m_position.xy / size;
	Vec4 color = texture2D(g_light, uv);
	color.a = 1.0;
	// The light buffer is linear; encode to sRGB unless the target does it.
#if !defined(ENABLE_TARGET_SRGB)
	color = to_srgb_space(color);
#endif
	return color;
}
