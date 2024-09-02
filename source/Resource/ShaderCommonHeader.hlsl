R"HLSLCODE(
#line 3 "ShaderCommonHeader.hlsl"
//////////////////////////////////////////////////////////////////////////////////////
#define IVec2 int2
#define IVec3 int3
#define IVec4 int4
#define IMat3 int3x3
#define IMat4 int4x4
#define Vec2 float2
#define Vec3 float3
#define Vec4 float4
#define Mat3 float3x3
#define Mat4 float4x4
#define DVec2 double2
#define DVec3 double3
#define DVec4 double4
#define DMat3 double3x3
#define DMat4 double4x4
#define Sampler2D(name) Texture2D name; SamplerState sempler_ ## name;
#define Sampler2DArray(name)  Texture2DArray name; SamplerState sempler_ ## name;
#define Sampler3D(name)  Texture3D name; SamplerState sempler_ ## name;
#define SamplerCube(name)  TextureCube name; SamplerState sempler_ ## name;
#define texture2D(name,pos) name.Sample(sempler_ ## name,pos)
#define texture2DArray(name,pos) name.Sample(sempler_ ## name,pos)
#define texture3D(name,pos) name.Sample(sempler_ ## name,pos)
#define textureCube(name,pos) name.Sample(sempler_ ## name,pos)

#ifdef GLSL_BACKEND
	#define shadow2D(name,pos) name.Sample(sempler_ ## name,invY(pos))
	#define drew2D(name,pos) name.Sample(sempler_ ## name,invY(pos))	

	#define shadow2DArray(name,pos) name.Sample(sempler_ ## name,invY(pos))
	#define drew2DArray(name,pos) name.Sample(sempler_ ## name,invY(pos))
	
	#define shadowCube(name,pos) name.Sample(sempler_ ## name,pos)
	#define drawCube(name,pos) name.Sample(sempler_ ## name,pos)
	static uint cube_map_view_id[]={ 0,1,2,3,4,5 };

	#define ONLY_GLSL( x ) x 
	#define ONLY_HLSL( x )
#else
	#define shadow2D(name,pos) name.Sample(sempler_ ## name, pos)
	#define drew2D(name,pos) name.Sample(sempler_ ## name, pos)

	#define shadow2DArray(name,pos) name.Sample(sempler_ ## name, pos)
	#define drew2DArray(name,pos) name.Sample(sempler_ ## name, pos)

	#define shadowCube(name,pos) name.Sample(sempler_ ## name,negY(pos))
	#define drawCube(name,pos) name.Sample(sempler_ ## name,negY(pos))
	static uint cube_map_view_id[]={ 0,1,3,2,4,5 };

	#define ONLY_GLSL( x ) 
	#define ONLY_HLSL( x ) x
#endif

////////////////////////////////////////////////////
inline Vec2 invX(in Vec2 pos)
{
	return Vec2(1.0 - pos.x, pos.y);
}
inline Vec2 invY(in Vec2 pos)
{
	return Vec2(pos.x, 1.0 - pos.y);
}

inline Vec3 invX(in Vec3 pos)
{
	return Vec3(1.0 - pos.x, pos.y, pos.z);
}
inline Vec3 invY(in Vec3 pos)
{
	return Vec3(pos.x, 1.0 - pos.y, pos.z);
}
inline Vec3 invZ(in Vec3 pos)
{
	return Vec3(pos.x, pos.y, 1.0 - pos.z);
}

inline Vec4 invX(in Vec4 pos)
{
	return Vec4(1.0 - pos.x, pos.y, pos.z, pos.w);
}
inline Vec4 invY(in Vec4 pos)
{
	return Vec4(pos.x, 1.0 - pos.y, pos.z, pos.w);
}
inline Vec4 invZ(in Vec4 pos)
{
	return Vec4(pos.x, pos.y, 1.0 - pos.z, pos.w);
}
inline Vec4 invW(in Vec4 pos)
{
	return Vec4(pos.x, pos.y, pos.z, 1.0 - pos.w);
}
////////////////////////////////////////////////////
inline Vec2 negX(in Vec2 pos)
{
	return Vec2(-pos.x, pos.y);
}
inline Vec2 negY(in Vec2 pos)
{
	return Vec2(pos.x, -pos.y);
}

inline Vec3 negX(in Vec3 pos)
{
	return Vec3(-pos.x, pos.y, pos.z);
}
inline Vec3 negY(in Vec3 pos)
{
	return Vec3(pos.x, -pos.y, pos.z);
}
inline Vec3 negZ(in Vec3 pos)
{
	return Vec3(pos.x, pos.y, -pos.z);
}

inline Vec4 negX(in Vec4 pos)
{
	return Vec4(-pos.x, pos.y, pos.z, pos.w);
}
inline Vec4 negY(in Vec4 pos)
{
	return Vec4(pos.x, -pos.y, pos.z, pos.w);
}
inline Vec4 negZ(in Vec4 pos)
{
	return Vec4(pos.x, pos.y, -pos.z, pos.w);
}
inline Vec4 negW(in Vec4 pos)
{
	return Vec4(pos.x, pos.y, pos.z, -pos.w);
}

////////////////////////////////////////////////////
Vec2 _SQUARE_textureSize2D(Texture2D tex2D, uint Level)
{
	uint  param;
	uint2 ret;
	tex2D.GetDimensions(Level, ret.x, ret.y, param);
	return Vec2(ret.x, ret.y);
}
#define textureSize2D(name,lod) _SQUARE_textureSize2D(name, lod)

Vec3 _SQUARE_textureSize3D(Texture3D tex3D, uint Level)
{
	uint  param;
	uint3 ret;
	tex3D.GetDimensions(Level, ret.x, ret.y, ret.z, param);
	return Vec3(ret.x, ret.y, ret.z);
}
#define textureSize3D(name,lod) _SQUARE_textureSize3D(name, lod)

Vec2 _SQUARE_textureSizeCube(TextureCube texCUBE, uint Level)
{
	uint  param;
	uint2 ret;
	texCUBE.GetDimensions(Level, ret.x, ret.y, param);
	return Vec2(ret.x, ret.y);
}
#define textureSizeCube(name,lod) _SQUARE_textureSizeCube(name, lod)
//////////////////////////////////////////////////////////////////////////////////////
)HLSLCODE"