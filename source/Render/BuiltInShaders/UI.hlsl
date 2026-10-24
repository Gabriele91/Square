R"HLSLCODE(
#line 3 "UI.hlsl"
//
//  UI.hlsl
//  Square
//
//  The geometry of RmlUi (see Backend.cpp): vertices in pixels of the window, moved by the
//  translation of the draw and by the transform of the element (RCSS transform), colors and
//  textures with premultiplied alpha.
//
struct UIVertex
{
	Vec2 m_position : POSITION;
	Vec4 m_color    : COLOR0;
	Vec2 m_uv       : TEXCOORD0;
};

struct UIOutput
{
	Vec4 m_position : SV_POSITION;
	Vec4 m_color    : COLOR0;
	Vec2 m_uv       : TEXCOORD0;
};

Sampler2D(ui_texture);
Vec2 ui_size;        //pixels of the window
Vec2 ui_translation; //pixels
Mat4 ui_transform;   //of the element (identity without a transform)

UIOutput vertex(UIVertex input)
{
	UIOutput output;
	Vec4 position = mul(Vec4(input.m_position + ui_translation, 0.0, 1.0), ui_transform);
	position.xy /= position.w;
	output.m_position = Vec4(position.x / ui_size.x * 2.0 - 1.0, 1.0 - position.y / ui_size.y * 2.0, 0.0, 1.0);
	output.m_color = input.m_color;
	output.m_uv = input.m_uv;
	return output;
}

Vec4 fragment(UIOutput input) : SV_TARGET0
{
	Vec4 color = input.m_color * texture2D(ui_texture, input.m_uv);
#if defined(ENABLE_GAMMA_CORRECTION) && defined(ENABLE_TARGET_SRGB)
	//RCSS colors and images are sRGB, the target encodes linear colors (premultiplied: the
	//color without its alpha)
	if (color.a > 0.0) color.rgb = pow(abs(color.rgb / color.a), 2.2) * color.a;
#endif
	return color;
}
)HLSLCODE"
