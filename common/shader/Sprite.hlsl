//
//  Sprite.hlsl
//  Square
//
//  Sprites (Scene::Sprite, Scene::ParticleEmitter: Render::SpriteBatch): a quad each, its
//  instances in the buffer "Sprites" (three vectors each: its center in the space of its actor
//  and its rotation; its size, its frame, w 1 the frame a share of them; its color), turned
//  toward the camera (axis.w 0: its rotation around the view) or around an axis (axis.w 1:
//  axis.xyz in the space of the actor, its face toward the camera: spray, fire). Unlit: its texture (albedo_map) by its color and by the
//  color of the material; the frame of a flipbook its own (>= 0) or by the time
//  (AnimatedUV: flipbook, uv_scroll).
//
#include <Camera>
#include <Transform>
#include <Vertex>
#include <Support>
#include <GammaCorrection>
#include <AnimatedUV>

#define SPRITES_MAX 256

cbuffer Sprites
{
	Vec4 sprites_data[SPRITES_MAX * 3];
};

Sampler2D(albedo_map);
Vec4 color;
Vec4 axis;

struct SpriteVSOutput
{
	Vec4 m_position : SV_POSITION;
	Vec2 m_uv       : TEXCOORD0;
	Vec4 m_color    : TEXCOORD1;
};

SpriteVSOutput vertex(Position3D input, uint instance_id : SV_InstanceID)
{
	const Vec4 center_rotation = sprites_data[instance_id * 3];
	const Vec4 size_frame = sprites_data[instance_id * 3 + 1];
	const Vec4 tint = sprites_data[instance_id * 3 + 2];
	const Vec3 center = mul(Vec4(center_rotation.xyz, 1.0), transform.m_model).xyz;
	//its two axes on the screen: of the camera (turned by its rotation), or the axis and the side
	//toward the camera
	Vec3 right = mul(Vec4(1.0, 0.0, 0.0, 0.0), camera.m_model).xyz;
	Vec3 up = mul(Vec4(0.0, 1.0, 0.0, 0.0), camera.m_model).xyz;
	if (axis.w > 0.5)
	{
		up = normalize(mul(Vec4(axis.xyz, 0.0), transform.m_model).xyz);
		right = normalize(cross(up, camera.m_position - center));
	}
	else
	{
		const float s = sin(center_rotation.w);
		const float c = cos(center_rotation.w);
		const Vec3 turned_right = right * c + up * s;
		up = up * c - right * s;
		right = turned_right;
	}
	const Vec2 corner = input.m_position.xy;
	const Vec3 world = center + right * (corner.x * 0.5 * size_frame.x) + up * (corner.y * 0.5 * size_frame.y);
	SpriteVSOutput output;
	output.m_position = mul_view_projection(world);
	//its uv (v down), the frame of its flipbook: its own, or by the time
	const Vec2 uv = Vec2(corner.x * 0.5 + 0.5, 0.5 - corner.y * 0.5);
	if (size_frame.z >= 0.0)
	{
		const float columns = max(flipbook.x, 1.0);
		const float rows = max(flipbook.y, 1.0);
		const float frames = flipbook.w > 0.0 ? flipbook.w : columns * rows;
		//its frame, or a share of the frames (w 1: a particle, its life)
		const float index = size_frame.w > 0.5 ? saturate(size_frame.z) * frames * 0.9999 : size_frame.z;
		const float frame = fmod(floor(index), frames);
		const Vec2  cell = Vec2(fmod(frame, columns), floor(frame / columns));
		output.m_uv = (uv + cell) / Vec2(columns, rows);
	}
	else
	{
		output.m_uv = animated_uv(uv);
	}
	output.m_color = tint * color;
	return output;
}

Vec4 fragment(SpriteVSOutput input) : SV_TARGET0
{
	Vec4 result = to_rgb_space(texture2D(albedo_map, input.m_uv)) * input.m_color;
#if !defined(ENABLE_TARGET_SRGB) && !defined(OUTPUT_LINEAR_COLOR)
	result = to_srgb_space(result);
#endif
	return result;
}
