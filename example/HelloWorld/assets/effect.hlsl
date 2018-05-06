
struct VS_INPUT
{
	Vec3 vPos   : POSITION;
	Vec3 vColor : COLOR0;
};

struct VS_OUT
{
	Vec4 Position : SV_POSITION;  // interpolated vertex position (system value)
	Vec4 Color    : COLOR0;       // interpolated diffuse color
};

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	Mat4 mWorld;      // world matrix for object
	Mat4 View;        // view matrix
	Mat4 Projection;  // projection matrix
};
#line 1
Vec4 g_uniform;

VS_OUT vertex(VS_INPUT input) 
{    
	VS_OUT Output;

	Vec4 pos = mul(g_uniform,float4(input.vPos, 1.0f));

	// Transform the position from object space to homogeneous projection space
	pos = mul(pos, mWorld);
	pos = mul(pos, View);
	pos = mul(pos, Projection);
	Output.Position = pos;

	// Just pass through the color data
	Output.Color = float4(input.vColor, 1.0f);

	return Output;
}

Vec4 fragment(VS_OUT input) : SV_Target
{    
	return input.Color;
}