//
//  Fog.hlsl
//  Square
//
//  The height fog (see PostEffectFog): the density along the way from the camera to the world
//  position of the pixel (G-Buffer), integrated in closed form (exponential height fog: the
//  density falls as e^(-falloff * (y - height))), the color of the fog brighter toward the sun.
//  The background (no geometry): the fog the sky setting says.
//
#include <Camera>
#include <GBufferPosition>
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_source);
Sampler2D(g_position);
Vec2 fog_size;          //pixels of the frame
Vec4 fog_color;         //rgb, the fog of the background
Vec4 fog_params;        //density, height, falloff, start (world units without fog)
Vec4 fog_sun_color;     //rgb, exponent of the glow
Vec4 fog_sun_direction; //where the light of the sun goes (normalized), max opacity

//the density integrated from the camera to a point, after the start
float fog_amount(in Vec3 from, in Vec3 to)
{
	float density  = fog_params.x;
	float height   = fog_params.y;
	float falloff  = fog_params.z;
	float start    = fog_params.w;
	float distance = length(to - from);
	float travel   = max(distance - start, 0.0);
	//the way in the fog begins at start
	Vec3  begin = from + (to - from) * (distance > 0.0001 ? min(start / distance, 1.0) : 0.0);
	//the density at its beginning, then its shape along the height (1 when flat)
	float density_begin = density * exp(clamp(-falloff * (begin.y - height), -40.0, 40.0));
	float k = clamp(falloff * (to.y - begin.y), -40.0, 40.0);
	float shape = abs(k) > 0.0001 ? (1.0 - exp(-k)) / k : 1.0;
	return 1.0 - exp(-density_begin * travel * shape);
}

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / fog_size;
	Vec3 color = texture2DLod(g_source, uv, 0.0).rgb;
	Vec4 g_pos = gbuffer_world(texture2DLod(g_position, uv, 0.0), uv);
	float max_opacity = fog_sun_direction.w;
	//background: the fog of the sky
	if (g_pos.w < 0.5) return Vec4(lerp(color, fog_color.rgb, fog_color.a * max_opacity), 1.0);
	//the glow of the sun, looking toward it
	Vec3  view = normalize(g_pos.xyz - camera.m_position);
	float sun = pow(saturate(dot(view, -fog_sun_direction.xyz)), fog_sun_color.w);
	Vec3  fog = fog_color.rgb + fog_sun_color.rgb * sun;
	float amount = min(fog_amount(camera.m_position, g_pos.xyz), max_opacity);
	return Vec4(lerp(color, fog, amount), 1.0);
}
