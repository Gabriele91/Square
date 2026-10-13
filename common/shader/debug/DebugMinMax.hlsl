//
//  DebugMinMax.hlsl
//  Square
//
//  The smallest and the largest value of a texture (the thumbnails of the debug panel, a value
//  normalized between them): each pixel of the target the smallest (r) and the largest (g) of a
//  block of texels of the source (reduce.xy texels, at most 16 x 16; reduce.zw: the size of the
//  source). The source holds one value (params.x: its red) or, a level before, the smallest and
//  the largest (r, g).
//
#include <Vertex>

Sampler2D(g_texture);

Vec4 reduce;
Vec4 params;

struct DebugMinMaxVSOutput
{
	Vec4 m_position : SV_POSITION;
};

DebugMinMaxVSOutput vertex(Position3D input)
{
	DebugMinMaxVSOutput output;
	output.m_position = Vec4(input.m_position.xy, 0.0, 1.0);
	return output;
}

Vec4 fragment(DebugMinMaxVSOutput input) : SV_TARGET0
{
	const Vec2 first = floor(input.m_position.xy) * reduce.xy;
	float smallest = 3.0e38;
	float largest = -3.0e38;
	for (int y = 0; y < 16; ++y)
	{
		for (int x = 0; x < 16; ++x)
		{
			const Vec2 texel = first + Vec2(float(x), float(y)) + 0.5;
			const bool inside = float(x) < reduce.x && float(y) < reduce.y && texel.x < reduce.z && texel.y < reduce.w;
			if (inside)
			{
				const Vec4 value = texture2DLod(g_texture, texel / reduce.zw, 0.0);
				smallest = min(smallest, value.r);
				largest = max(largest, params.x > 0.5 ? value.r : value.g);
			}
		}
	}
	return Vec4(smallest, largest, 0.0, 1.0);
}
