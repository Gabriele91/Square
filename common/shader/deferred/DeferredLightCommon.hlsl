//
//  DeferredLightCommon.hlsl
//  Square
//
//  Shared by every deferred light pass: G-Buffer samplers and the shading entry
//  that lights each pixel with the shading model stored in position.w (see
//  <SurfaceDeferredPBR>): PBR reuses compute_light from <LightPBR>, LEGACY
//  reuses the forward Blinn-Phong math (see <LightLegacy>).
//
//  Include AFTER the light type (RENDERING_*_LIGHT) and <LightPBR>, so that
//  compute_light() and the per-type attenuation/shadow helpers are available.
//
#pragma once

//G-Buffer inputs (bound by DrawerPassDeferred)
Sampler2D(g_position);
Sampler2D(g_normal);
Sampler2D(g_albedo);
Sampler2D(g_emissive);

//screen uv from SV_POSITION (same convention on every backend, no y flip)
Vec2 deferred_screen_uv(in Vec4 sv_position)
{
	return (sv_position.xy - camera.m_viewport.xy) / camera.m_viewport.zw;
}

//////////////////////////////////////////////////////////////////////
// Legacy (Blinn-Phong) lighting, one function per light type with the
// same math as the forward legacy renderer (see *LightLegacy.hlsl),
// reusing the attenuation/shadow helpers of the active light type.
//////////////////////////////////////////////////////////////////////
//output
struct LegacyLightResult
{
	Vec3 m_diffuse;
	Vec3 m_specular;
};

#if defined(RENDERING_AMBIENT_LIGHT)
//forward: AmbientLightLegacy
LegacyLightResult legacy_compute_light
(
	 in Vec4  fposition,
	 in Vec3  view_dir,
	 in Vec3  normal,
	 in float occlusion,
	 in float shininess
)
{
	// Combine results
	LegacyLightResult result;
	result.m_diffuse  = light.rgb * occlusion;
	result.m_specular = Vec3(1.0, 1.0, 1.0);
	//return
	return result;
}
#elif defined(RENDERING_DIRECTION_LIGHT)
//forward: DirectionLightLegacy
LegacyLightResult legacy_compute_light
(
	 in Vec4  fposition,
	 in Vec3  view_dir,
	 in Vec3  normal,
	 in float occlusion,
	 in float shininess
)
{
	// Light dir
	Vec3 light_dir = normalize(-light.m_direction);
	// Diffuse shading
	float diff = max(dot(normal, light_dir), 0.0);
	// Specular shading
	Vec3  halfway_dir = normalize(light_dir + view_dir);
	float spec = pow(max(dot(normal, halfway_dir), 0.0), shininess);
	// Apply shadow
	float shadow_factor = direction_light_apply_shadow(fposition, light_dir, normal);
	// Combine results
	LegacyLightResult result;
	result.m_diffuse  = light.m_diffuse  * diff * shadow_factor;
	result.m_specular = light.m_specular * spec * shadow_factor;
	//return
	return result;
}
#elif defined(RENDERING_POINT_LIGHT)
//forward: PointLightLegacy
LegacyLightResult legacy_compute_light
(
	 in Vec4  fposition,
	 in Vec3  view_dir,
	 in Vec3  normal,
	 in float occlusion,
	 in float shininess
)
{
	//value return
	LegacyLightResult result;
	// Light relative
	Vec3 light_relative = light.m_position - fposition.xyz;
	// Attenuation
	float attenuation = point_light_compute_attenuation(light_relative);
	// Exit case
	if (attenuation <= 0.0)
	{
		result.m_diffuse  = Vec3(0.0, 0.0, 0.0);
		result.m_specular = Vec3(0.0, 0.0, 0.0);
		return result;
	}
	// Light dir
	Vec3 light_dir = normalize(light_relative);
	// Diffuse shading
	float diff = max(dot(normal, light_dir), 0.0);
	// Specular shading
	Vec3  halfway_dir = normalize(light_dir + view_dir);
	float spec = pow(max(dot(normal, halfway_dir), 0.0), shininess);
	// Apply shadow
	float shadow_factor = point_light_apply_shadow(fposition);
	// Combine results
	result.m_diffuse  = light.m_diffuse  * diff * attenuation * shadow_factor;
	result.m_specular = light.m_specular * spec * attenuation * shadow_factor;
	//return
	return result;
}
#elif defined(RENDERING_SPOT_LIGHT)
//forward: SpotLightLegacy
LegacyLightResult legacy_compute_light
(
	 in Vec4  fposition,
	 in Vec3  view_dir,
	 in Vec3  normal,
	 in float occlusion,
	 in float shininess
)
{
	//value return
	LegacyLightResult result;
	// Light relative
	Vec3 light_relative = light.m_position - fposition.xyz;
	// Attenuation
	float attenuation = spot_light_compute_attenuation(light_relative);
	// Exit case
	if (attenuation <= 0.0)
	{
		result.m_diffuse  = Vec3(0.0, 0.0, 0.0);
		result.m_specular = Vec3(0.0, 0.0, 0.0);
		return result;
	}
	// Light dir
	Vec3 light_dir = normalize(light_relative);
	// Diffuse shading
	float diff = max(dot(normal, light_dir), 0.0);
	// Specular shading
	Vec3  halfway_dir = normalize(light_dir + view_dir);
	float spec = pow(max(dot(normal, halfway_dir), 0.0), shininess);
	// Spotlight intensity
	float theta = dot(light_dir, normalize(-light.m_direction));
	float epsilon = light.m_inner_cut_off - light.m_outer_cut_off;
	float intensity = clamp((theta - light.m_outer_cut_off) / epsilon, 0.0, 1.0);
	// Apply shadow
	float shadow_factor = spot_light_apply_shadow(fposition);
	// Combine results
	result.m_diffuse  = light.m_diffuse  * diff * attenuation * intensity * shadow_factor;
	result.m_specular = light.m_specular * spec * attenuation * intensity * shadow_factor;
	//return
	return result;
}
#endif

//legacy shading: light the pixel and combine like SurfaceForwardLegacy
Vec3 deferred_shade_legacy
(
	  in Vec4  fposition
	, in Vec3  view_dir
	, in Vec3  normal
	, in Vec3  albedo
	, in Vec3  specular_color
	, in float occlusion
	, in float shininess
)
{
	LegacyLightResult result = legacy_compute_light(fposition, view_dir, normal, occlusion, shininess);
	return albedo * (result.m_diffuse + result.m_specular * specular_color);
}

//rebuild the surface from the G-Buffer and evaluate the current light with the
//per-pixel shading model
Vec4 deferred_shade(in Vec2 uv)
{
	Vec4 gbuffer_position = texture2D(g_position, uv);
	//background: no geometry was written here
	if (gbuffer_position.w < 0.5)
	{
		return Vec4(0.0, 0.0, 0.0, 0.0);
	}
	Vec4 gbuffer_normal   = texture2D(g_normal,   uv);
	Vec4 gbuffer_albedo   = texture2D(g_albedo,   uv);
	Vec4 gbuffer_emissive = texture2D(g_emissive, uv);
	//view direction (same as the forward surface shaders)
	Vec3 view_dir = normalize(camera.m_position - gbuffer_position.xyz);
	//LEGACY (Blinn-Phong)
	if (gbuffer_position.w > 1.5)
	{
		Vec3 color = deferred_shade_legacy
		(
			  Vec4(gbuffer_position.xyz, 1.0)
			, view_dir
			, normalize(gbuffer_normal.xyz)
			, gbuffer_albedo.rgb    //albedo
			, gbuffer_emissive.rgb  //specular color
			, gbuffer_emissive.a    //occlusion
			, gbuffer_normal.w      //shininess
		);
		return Vec4(color, 1.0);
	}
	//PBR
	SurfaceData data = decode_gbuffer(gbuffer_position, gbuffer_normal, gbuffer_albedo, gbuffer_emissive);
	LightResult result = compute_light(view_dir, data);
	return Vec4(result.m_radiance, 1.0);
}
