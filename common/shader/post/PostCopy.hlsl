//
//  PostCopy.hlsl
//  Square
//
//  Draws a texture of the frame size as it is (full-screen): the final blit of the forward
//  pipeline with post effects (its offscreen target holds the forward output, already in the
//  space of the screen: linear with an sRGB framebuffer, encoded by the shaders otherwise).
//
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_source);

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 size = textureSize2D(g_source, 0);
	Vec4 color = texture2D(g_source, input.m_position.xy / size);
	color.a = 1.0;
	return color;
}
