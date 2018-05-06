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