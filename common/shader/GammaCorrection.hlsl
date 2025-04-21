#pragma once
#define ENABLE_GAMMA_CORRECTION 1

float to_srgb_space(in float color)
{
#if ENABLE_GAMMA_CORRECTION
   static const float inv_gamma = 1.0 / 2.2;
   return pow(color, inv_gamma);
#else
    return color;
#endif
}

Vec2 to_srgb_space(in Vec2 color)
{
#if ENABLE_GAMMA_CORRECTION
   static const float inv_gamma = 1.0 / 2.2;
   return pow(color, Vec2(inv_gamma,inv_gamma));
#else
    return color;
#endif
}

Vec3 to_srgb_space(in Vec3 color)
{
#if ENABLE_GAMMA_CORRECTION
   static const float inv_gamma = 1.0 / 2.2;
   return pow(color, Vec3(inv_gamma,inv_gamma,inv_gamma));
#else
    return color;
#endif
}

Vec4 to_srgb_space(in Vec4 color)
{
#if ENABLE_GAMMA_CORRECTION
    // Apply gamma correction only to RGB, not the alpha channel
    color.rgb = to_srgb_space(color.rgb);
    return color;
#else
    return color;
#endif
}

float to_rgb_space(in float color)
{
#if ENABLE_GAMMA_CORRECTION
   static const float gamma = 2.2;
   return pow(color, gamma);
#else
    return color;
#endif
}

Vec2 to_rgb_space(in Vec2 color)
{
#if ENABLE_GAMMA_CORRECTION
   static const float gamma = 2.2;
   return pow(color, Vec2(gamma,gamma));
#else
    return color;
#endif
}

Vec3 to_rgb_space(in Vec3 color)
{
#if ENABLE_GAMMA_CORRECTION
   static const float gamma = 2.2;
   return pow(color, Vec3(gamma,gamma,gamma));
#else
    return color;
#endif
}

Vec4 to_rgb_space(in Vec4 color)
{
#if ENABLE_GAMMA_CORRECTION
    // Apply gamma correction only to RGB, not the alpha channel
    color.rgb = to_rgb_space(color.rgb);
    return color;
#else
    return color;
#endif
}
