R"HLSLCODE(
#line 3 "SplashScreen.hlsl"
//
//  SplashScreen.hlsl
//  Square
//
//  The icon of the splash screen (see SplashScreen.cpp): the quad of BasicMesh::build_quad
//  scaled and moved in the window, the image with its alpha over the background.
//
struct SplashVertex
{
	Vec3 m_position : POSITION;
};

struct SplashOutput
{
	Vec4 m_position : SV_POSITION;
	Vec2 m_uv       : TEXCOORD0;
};

Sampler2D(g_splash);
Vec4 splash_rect; //the quad in NDC: scale (xy), offset (zw)

SplashOutput vertex(SplashVertex input)
{
	SplashOutput output;
	output.m_position = Vec4(input.m_position.xy * splash_rect.xy + splash_rect.zw, 0.0, 1.0);
	//the first row of the image at the top
	output.m_uv = Vec2(input.m_position.x * 0.5 + 0.5, 0.5 - input.m_position.y * 0.5);
	return output;
}

Vec4 fragment(SplashOutput input) : SV_TARGET0
{
	Vec4 color = texture2D(g_splash, input.m_uv);
#if defined(ENABLE_GAMMA_CORRECTION) && defined(ENABLE_TARGET_SRGB)
	//the image is sRGB, the target encodes linear colors
	color.rgb = pow(abs(color.rgb), 2.2);
#endif
	return color;
}
)HLSLCODE"
