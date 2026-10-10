//
//  DebugTextureCommon.hlsl
//  Square
//
//  The color of a texel of a texture shown for debug (DebugTexture2D, Array, Cube): as it is;
//  one value (params.x: a depth, a texture of a channel) in grey; or that value between the
//  smallest and the largest of the texture (params.w: g_minmax, its 1x1 texel holds them,
//  DebugMinMax) in the colors of the Turbo map (dark blue the smallest, dark red the largest).
//  Include it after rect and params.
//
#pragma once

Sampler2D(g_minmax);

//the Turbo color map of a value in [0, 1] (its polynomial fit)
Vec3 debug_turbo(in float t)
{
	const Vec4 r4 = Vec4(0.13572138, 4.61539260, -42.66032258, 132.13108234);
	const Vec4 g4 = Vec4(0.09140261, 2.19418839, 4.84296658, -14.18503333);
	const Vec4 b4 = Vec4(0.10667330, 12.64194608, -60.58204836, 110.36276771);
	const Vec2 r2 = Vec2(-152.94239396, 59.28637943);
	const Vec2 g2 = Vec2(4.27729857, 2.82956604);
	const Vec2 b2 = Vec2(-89.90310912, 27.34824973);
	const float x = saturate(t);
	const Vec4 v4 = Vec4(1.0, x, x * x, x * x * x);
	const Vec2 v2 = v4.zw * v4.z;
	return Vec3(dot(v4, r4) + dot(v2, r2), dot(v4, g4) + dot(v2, g2), dot(v4, b4) + dot(v2, b2));
}

//the color shown of a texel
Vec4 debug_texture_color(in Vec4 color)
{
	Vec4 shown = Vec4(color.rgb, 1.0);
	if (params.w > 0.5)
	{
		const Vec2  range = texture2DLod(g_minmax, Vec2(0.5, 0.5), 0.0).rg;
		const float t = (color.r - range.x) / max(range.y - range.x, 1e-8);
		shown = Vec4(debug_turbo(t), 1.0);
	}
	else if (params.x > 0.5)
	{
		shown = Vec4(color.rrr, 1.0);
	}
	return shown;
}
