//
//  DeferredLightCommon.hlsl
//  Square
//
//  Shared bits for every deferred light pass: the G-Buffer samplers and the
//  shading entry that rebuilds the surface and lights it with the shading model
//  stored per pixel in the G-Buffer (position.w, see <SurfaceDeferredPBR>):
//   - PBR    : reuses the *same* PBR light math as the forward renderer
//              (compute_light from <LightPBR>);
//   - LEGACY : reuses the *same* Blinn-Phong math as the forward legacy
//              renderer (diffuse/specular below), including the specular color
//              stored in GT3.
//
//  Must be included AFTER the light type has been selected (RENDERING_*_LIGHT)
//  and <LightPBR> has been included, so that compute_light() and the per-type
//  attenuation/shadow helpers are available.
//
#pragma once

// G-Buffer inputs (bound by DrawerPassDeferred).
Sampler2D(g_position);
Sampler2D(g_normal);
Sampler2D(g_albedo);
Sampler2D(g_emissive);

// Screen UV from the fragment SV_POSITION (pixel coordinates): both the
// G-Buffer and the light buffer are rasterized with the same convention on
// every backend, so no Y flip is ever needed.
Vec2 deferred_screen_uv(in Vec4 sv_position)
{
	return (sv_position.xy - camera.m_viewport.xy) / camera.m_viewport.zw;
}

// Legacy (Blinn-Phong) lighting: same math as the forward legacy renderer
// (see shader/legacy/*LightLegacy.hlsl); reuses the attenuation and shadow
// helpers of the active light type declared by the included PBR light file.
Vec3 deferred_shade_legacy
(
	  in Vec4  fposition
	, in Vec3  view_dir
	, in Vec3  normal
	, in Vec3  albedo
	, in Vec3  spec_color
	, in float occlusion
	, in float shininess
)
{
	Vec3 diffuse  = Vec3(0.0, 0.0, 0.0);
	Vec3 specular = Vec3(0.0, 0.0, 0.0);
#if defined(RENDERING_AMBIENT_LIGHT)
	// forward: AmbientLightLegacy
	diffuse  = light.rgb * occlusion;
	specular = Vec3(1.0, 1.0, 1.0);
#elif defined(RENDERING_DIRECTION_LIGHT)
	// forward: DirectionLightLegacy
	Vec3  light_dir   = normalize(-light.m_direction);
	float diff        = max(dot(normal, light_dir), 0.0);
	Vec3  halfway_dir = normalize(light_dir + view_dir);
	float spec        = pow(max(dot(normal, halfway_dir), 0.0), shininess);
	float shadow      = direction_light_apply_shadow(fposition, light_dir, normal);
	diffuse  = light.m_diffuse  * diff * shadow;
	specular = light.m_specular * spec * shadow;
#elif defined(RENDERING_POINT_LIGHT)
	// forward: PointLightLegacy
	Vec3  light_relative = light.m_position - fposition.xyz;
	float attenuation    = point_light_compute_attenuation(light_relative);
	if (attenuation > 0.0)
	{
		Vec3  light_dir   = normalize(light_relative);
		float diff        = max(dot(normal, light_dir), 0.0);
		Vec3  halfway_dir = normalize(light_dir + view_dir);
		float spec        = pow(max(dot(normal, halfway_dir), 0.0), shininess);
		float shadow      = point_light_apply_shadow(fposition);
		diffuse  = light.m_diffuse  * diff * attenuation * shadow;
		specular = light.m_specular * spec * attenuation * shadow;
	}
#elif defined(RENDERING_SPOT_LIGHT)
	// forward: SpotLightLegacy
	Vec3  light_relative = light.m_position - fposition.xyz;
	float attenuation    = spot_light_compute_attenuation(light_relative);
	if (attenuation > 0.0)
	{
		Vec3  light_dir   = normalize(light_relative);
		float diff        = max(dot(normal, light_dir), 0.0);
		Vec3  halfway_dir = normalize(light_dir + view_dir);
		float spec        = pow(max(dot(normal, halfway_dir), 0.0), shininess);
		float theta       = dot(light_dir, normalize(-light.m_direction));
		float epsilon     = light.m_inner_cut_off - light.m_outer_cut_off;
		float intensity   = clamp((theta - light.m_outer_cut_off) / epsilon, 0.0, 1.0);
		float shadow      = spot_light_apply_shadow(fposition);
		diffuse  = light.m_diffuse  * diff * attenuation * intensity * shadow;
		specular = light.m_specular * spec * attenuation * intensity * shadow;
	}
#endif
	// forward: SurfaceForwardLegacy combine
	return albedo * (diffuse + specular * spec_color);
}

// Rebuild the surface from the G-Buffer and evaluate the current light with the
// per-pixel shading model.
Vec4 deferred_shade(in Vec2 uv)
{
	Vec4 g_pos = texture2D(g_position, uv);
	// Background: no geometry was written here (the clear leaves the shading
	// model id at GBUFFER_MODEL_BACKGROUND).
	if (g_pos.w < 0.5)
	{
		return Vec4(0.0, 0.0, 0.0, 0.0);
	}
	Vec4 g_norm  = texture2D(g_normal,   uv);
	Vec4 g_alb   = texture2D(g_albedo,   uv);
	Vec4 g_extra = texture2D(g_emissive, uv);
	// View direction (same as the forward surface shaders)
	Vec3 view_direction = normalize(camera.m_position - g_pos.xyz);
	// LEGACY (Blinn-Phong)
	if (g_pos.w > 1.5)
	{
		Vec3 color = deferred_shade_legacy(
			  Vec4(g_pos.xyz, 1.0)
			, view_direction
			, normalize(g_norm.xyz)
			, g_alb.rgb    // albedo
			, g_extra.rgb  // specular color
			, g_extra.a    // occlusion
			, g_norm.w     // shininess
		);
		return Vec4(color, 1.0);
	}
	// PBR
	SurfaceData data = decode_gbuffer(g_pos, g_norm, g_alb, g_extra);
	LightResult result = compute_light(view_direction, data);
	return Vec4(result.m_radiance, 1.0);
}
