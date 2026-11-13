//
//  DeferredDecal.hlsl
//  Square
//
//  Deferred pass of a decal (Render::Decal): a volume around its box, the position of the G-Buffer
//  in it taken into its box (-1 to 1); inside it its texture on its xz (projected along its y),
//  its color added as it is (emissive), fading at the top and the bottom of its box.
//
#include <Camera>
#include <Vertex>
#include <GBufferPosition>
#include <DeferredVolume>

//the G-Buffer: the position (bound by DrawerPassDeferred)
Sampler2D(g_position);
//its texture
Sampler2D(decal_map);

struct DecalStruct
{
	Mat4 m_inverse; //the world to its box
	Vec4 m_color;
};

cbuffer Decal
{
	DecalStruct decal;
};

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2 uv = input.m_position.xy / textureSize2D(g_position, 0);
	Vec4 position = gbuffer_world(texture2D(g_position, uv), uv);
	Vec4 color = Vec4(0.0, 0.0, 0.0, 0.0);
	//a surface, in its box
	Vec3 local = mul(Vec4(position.xyz, 1.0), decal.m_inverse).xyz;
	bool inside = abs(local.x) <= 1.0 && abs(local.y) <= 1.0 && abs(local.z) <= 1.0;
	if (position.w > 0.5 && inside)
	{
		Vec4 texel = texture2D(decal_map, local.xz * 0.5 + 0.5);
		float fade = 1.0 - smoothstep(0.75, 1.0, abs(local.y));
		color = Vec4(decal.m_color.rgb * texel.rgb * (texel.a * decal.m_color.a * fade), 1.0);
	}
	return color;
}
