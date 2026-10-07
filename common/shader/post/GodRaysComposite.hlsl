//
//  GodRaysComposite.hlsl
//  Square
//
//  Last pass of the god rays (see PostEffectGodRays): the frame plus the rays times their color
//  and intensity, less as the sun goes out of the view.
//
#include <Camera>
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_source);
Sampler2D(g_rays);
Vec2 rays_size;  //pixels of the frame
Vec4 rays_sun;   //toward the sun, fade
Vec4 rays_color; //rgb, intensity
#include <GodRaysCommon>

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / rays_size;
	Vec3 color = texture2DLod(g_source, uv, 0.0).rgb;
	float w;
	Vec2  sun = rays_sun_uv(w);
	float visible = rays_sun_visibility(sun, w);
	Vec3  rays = texture2DLod(g_rays, uv, 0.0).rgb;
	return Vec4(color + rays * rays_color.rgb * rays_color.a * visible, 1.0);
}
