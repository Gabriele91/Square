Vec3 calculate_PBR(in Vec3 albedo, 
                   float metallic,
                   float roughness, 
                   in Vec3 normal, 
                   in Vec3 view_direction, 
                   in Vec3 light_relative, 
                   in Vec3 light_color) 
{
    const float PRB_EPSILON = 0.0001;
    const float PRB_PLASTIC = 0.04;
    const float PRB_MIN_ROUGHNESS = 0.01;

    // Clamp roughness
    roughness = clamp(roughness, PRB_MIN_ROUGHNESS, 1.0);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    Vec3 plastic = Vec3(PRB_PLASTIC,PRB_PLASTIC,PRB_PLASTIC);
    Vec3 v3_metallic = Vec3(metallic,metallic,metallic);
    Vec3 F0 = lerp(plastic, albedo, v3_metallic);
   
    // Light dir
    Vec3 light_direction = normalize(light_relative);
    // Half direction
    Vec3 half_direction = normalize(view_direction + light_direction);
    // Angle btw normal and camera
    float n_dot_v = max(dot(normal, view_direction), PRB_EPSILON);  
    // Angle btw normal and light
    float n_dot_l = max(dot(normal, light_direction), PRB_EPSILON);  
    // Angle btw middle and view
    float h_dot_v = max(dot(half_direction, view_direction), PRB_EPSILON);

    // PBR
    float NDF = min(distribution_GGX(normal, half_direction, roughness), NDF_PI);
    float G = geometry_smith(normal, view_direction, light_direction, roughness);
    Vec3 F = fresnel_schlick(h_dot_v, F0);
    Vec3 numerator = (NDF * G) * F;
    float denominator = 4.0 * n_dot_v * n_dot_l;
    Vec3 specular = numerator / denominator;

    // kS,kD
    // kS is equal to Fresnel
    Vec3 kS = F;
    // relationship the diffuse component (kD) should equal 1.0 - kS
    Vec3 kD = (1.0 - kS);
    // multiply kD by the inverse metalness such that only non-metals
    // have diffuse lighting
    kD *= (1.0 - metallic);
    
    // Radiance Lo
    // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    return (kD * albedo / NDF_PI + specular) * light_color * n_dot_l; 
}
