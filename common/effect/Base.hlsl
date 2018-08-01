////////////////
#include <Camera>
#include <Transform>
#include <Vertex>
#include <Support>
#include <Matrix>
#include <Surface>
////////////////
struct VertexShaderOutput
{
	Vec4 m_position : SV_POSITION;  // interpolated vertex position (system value)
	Mat3 m_normal   : NORMAL0;      // normal
	Vec2 m_uv       : TEXCOORD0;    // interpolated uv map
};

//global uniform
float mask;
float metallic;
float smoothness;
Vec4 color;

//texture
Sampler2D(diffuse_map);
Sampler2D(normal_map);
Sampler2D(specular_map);
Sampler2D(occlusion_map);

VertexShaderOutput vertex(Position3DNormalTangetBinomialUV input)
{
	VertexShaderOutput output;
	output.m_position = mul_model_view_projection(input.m_position);
	output.m_normal = compute_tbn(
		  compute_normal_matrix(transform.m_model)
		, input.m_normal
		, input.m_tangent
		, input.m_binomial
	);
	output.m_uv = input.m_uv;
	return output;
}

surface(VertexShaderOutput input)
{
	//diffuse/albedo
	Vec4 diffuse_color = texture2D(diffuse_map, input.m_uv);
	if (diffuse_color.a <= mask) discard;
	//material info
	Vec4 normal_color = texture2D(normal_map, input.m_uv);
	Vec4 specular_color = texture2D(specular_map, input.m_uv);
	Vec4 occlusion_color = texture2D(occlusion_map, input.m_uv);
	//output data
	SurfaceData data = DefaultSurfaceData();
	data.m_position = input.m_position.rgb;
	//color
	data.m_albedo = color.rgb * diffuse_color.rgb;
	data.m_alpha = color.a * diffuse_color.a;
	//norm info
	data.m_normal = mul(input.m_normal, normal_from_texture(normal_color));
	//material info
	data.m_specular = specular_color.rgb;
	data.m_occlusion = occlusion_color.r;
	//physics render info
	data.m_metallic = metallic;
	data.m_smoothness = smoothness;
	//return
	surface_return(data);
}