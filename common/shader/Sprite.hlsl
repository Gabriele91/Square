//
//  Sprite.hlsl
//  Square
//
//  Sprites (Scene::Sprite, Scene::ParticleEmitter: Render::SpriteBatch): a quad each, its
//  instances in the buffer "Sprites" (three vectors each: its center in the space of its actor
//  and its rotation; its size, its frame, w 1 the frame a share of them; its color), turned
//  toward the camera (axis.w 0: its rotation around the view) or around an axis (axis.w 1:
//  axis.xyz in the space of the actor, its face toward the camera: spray, fire). Its color: its
//  texture (albedo_map) by its color and by the color of the material; the frame of a flipbook
//  its own (>= 0) or by the time (AnimatedUV: flipbook, uv_scroll).
//
//  Its light (the passes of the effect: lights off, unlit; only_ambient; a light each, its shadow)
//  made for sprites, at each vertex (a quad, many of them over each other: no light a pixel): only
//  diffuse (no specular), wrapped (a volume, not a plane: its side away from the light lit a
//  little), through it toward the light (translucency: smoke, mist against the sun), its normal
//  toward the camera bent to its border (roundness); the shadow of the light at each pixel, by the
//  filter of the light as every surface (shadow_one_sample, its material: one sample), in the pixel
//  shader: a texture in the vertex shader takes the slots of the ones of the pixel shader (the
//  drivers bind a texture to the same slot of both).
//
#include <Camera>
#include <Transform>
#include <Vertex>
#include <Support>
#include <GammaCorrection>
#include <AnimatedUV>

#define SPRITES_MAX 256
//the side away from the light: lit by this share of it (0: a plane)
#define SPRITE_WRAP 0.5
//the light through it: sharper toward the light by this power
#define SPRITE_THROUGH_POWER 4.0

cbuffer Sprites
{
	Vec4 sprites_data[SPRITES_MAX * 3];
};

Sampler2D(albedo_map);
Vec4 color;
Vec4 axis;
float translucency;
float roundness;
float light_alpha_power;

//the diffuse of a sprite: wrapped, and through it toward the light
float sprite_diffuse(in Vec3 normal, in Vec3 to_light, in Vec3 to_camera)
{
	const float lit = saturate((dot(normal, to_light) + SPRITE_WRAP) / (1.0 + SPRITE_WRAP));
	const float through = translucency * pow(saturate(dot(-to_camera, to_light)), SPRITE_THROUGH_POWER);
	return lit + through;
}

//the light of each pass (its define: Effect.cpp), of the light shaders (DirectionLight, PointLight,
//SpotLight: their buffers, their shadows): at a vertex (sprite_light), its shadow at a pixel
//(sprite_shadow)
#if defined(SQ_LIGHT_AMBIENT)
Vec4 light;

Vec3 sprite_light(in Vec3 position, in Vec3 normal, in Vec3 to_camera)
{
	return light.rgb;
}

Vec3 sprite_shadow(in Vec3 position, in Vec3 normal)
{
	return Vec3(1.0, 1.0, 1.0);
}
#elif defined(SQ_LIGHT_DIRECTION)
#include <DirectionLight>

Vec3 sprite_light(in Vec3 position, in Vec3 normal, in Vec3 to_camera)
{
	return light.m_diffuse * sprite_diffuse(normal, normalize(-light.m_direction), to_camera);
}

Vec3 sprite_shadow(in Vec3 position, in Vec3 normal)
{
	const Vec4 fposition = Vec4(position, 1.0);
	return direction_light_apply_shadow(fposition, normalize(-light.m_direction), normal) * direction_light_shadow_tint(fposition);
}
#elif defined(SQ_LIGHT_POINT)
#include <PointLight>

Vec3 sprite_light(in Vec3 position, in Vec3 normal, in Vec3 to_camera)
{
	const Vec3 light_relative = light.m_position - position;
	return light.m_diffuse * (point_light_compute_attenuation(light_relative) * sprite_diffuse(normal, normalize(light_relative), to_camera));
}

Vec3 sprite_shadow(in Vec3 position, in Vec3 normal)
{
	return point_light_apply_shadow(Vec4(position, 1.0)).xxx;
}
#elif defined(SQ_LIGHT_SPOT)
#include <SpotLight>

Vec3 sprite_light(in Vec3 position, in Vec3 normal, in Vec3 to_camera)
{
	const Vec3 light_relative = light.m_position - position;
	const Vec3 light_direction = normalize(light_relative);
	const float cone = spot_light_compute_attenuation(light_relative) * spot_light_compute_penumbra(light_direction);
	return light.m_diffuse * (cone * sprite_diffuse(normal, light_direction, to_camera));
}

Vec3 sprite_shadow(in Vec3 position, in Vec3 normal)
{
	return spot_light_apply_shadow(Vec4(position, 1.0)).xxx;
}
#else
//lights off: unlit
Vec3 sprite_light(in Vec3 position, in Vec3 normal, in Vec3 to_camera)
{
	return Vec3(1.0, 1.0, 1.0);
}

Vec3 sprite_shadow(in Vec3 position, in Vec3 normal)
{
	return Vec3(1.0, 1.0, 1.0);
}
#endif

struct SpriteVSOutput
{
	Vec4 m_position : SV_POSITION;
	Vec2 m_uv       : TEXCOORD0;
	Vec4 m_color    : TEXCOORD1;
	Vec3 m_light    : TEXCOORD2;
	Vec3 m_world    : TEXCOORD3;
	Vec3 m_normal   : TEXCOORD4;
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
	//its light at this vertex: its normal toward the camera, bent to the corner (a volume)
	const Vec3 to_camera = normalize(camera.m_position - world);
	const Vec3 normal = normalize(normalize(camera.m_position - center) + normalize(right * corner.x + up * corner.y) * roundness);
	output.m_light = sprite_light(world, normal, to_camera);
	output.m_world = world;
	output.m_normal = normal;
	return output;
}

Vec4 fragment(SpriteVSOutput input) : SV_TARGET0
{
	const Vec4 albedo = to_rgb_space(texture2D(albedo_map, input.m_uv)) * input.m_color;
	Vec4 result = Vec4(albedo.rgb * (input.m_light * sprite_shadow(input.m_world, normalize(input.m_normal))), albedo.a);
#if defined(SQ_LIGHT_DIRECTION) || defined(SQ_LIGHT_POINT) || defined(SQ_LIGHT_SPOT)
	//a light added (its pass: blend src_alpha one) by its alpha to a power (light_alpha_power): 2,
	//only where it is thick (not its fringes, not the thin ones over each other: no light piled up,
	//no halo: smoke); 1, by its alpha (thin mist, spray: lit as they are)
	result.a = pow(saturate(albedo.a), max(light_alpha_power, 1.0));
#endif
#if !defined(ENABLE_TARGET_SRGB) && !defined(OUTPUT_LINEAR_COLOR)
	result = to_srgb_space(result);
#endif
	return result;
}
