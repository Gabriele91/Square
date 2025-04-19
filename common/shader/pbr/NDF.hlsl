// PI
static const float NDF_PI = 3.14159265359;
static const float NDF_MIN_EPSION = 0.0001;
// ----------------------------------------------------------------------------
Vec3 fresnel_schlick(float cos_theta, in Vec3 F0)
{
    Vec3 one_minus_F0 = 1.0 - F0;
    float inv_cos_theta = clamp(1.0 - cos_theta, NDF_MIN_EPSION, 1.0);
    float pow_result = pow(inv_cos_theta, 5.0);
    return F0 + one_minus_F0 * pow_result;
}
// ----------------------------------------------------------------------------
float distribution_GGX(in Vec3 N, in Vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), NDF_MIN_EPSION);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = NDF_PI * denom * denom;

    return nom / max(denom, NDF_MIN_EPSION);
}
// ----------------------------------------------------------------------------
float geometry_schlick_GGX(float n_dot_v, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = n_dot_v;
    float denom = n_dot_v * (1.0 - k) + k;

    return nom / max(denom, NDF_MIN_EPSION);
}
// ----------------------------------------------------------------------------
float geometry_smith(in Vec3 N, in Vec3 V, in Vec3 L, float roughness)
{
    float n_dot_v = max(dot(N, V), NDF_MIN_EPSION);
    float n_dot_l = max(dot(N, L), NDF_MIN_EPSION);
    float ggx2 = geometry_schlick_GGX(n_dot_v, roughness);
    float ggx1 = geometry_schlick_GGX(n_dot_l, roughness);

    return ggx1 * ggx2;
}