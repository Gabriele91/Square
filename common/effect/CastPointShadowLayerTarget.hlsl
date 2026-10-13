////////////////
// Multi-pass point-light shadow (cube map) for backends that can not route a draw to a layer:
// no geometry shaders, no layer written by the vertex shader (GLES 3.1, Vulkan without both).
// The host binds each face as the target and draws its casters there; multi_pass.m_id selects
// the view-projection only.
////////////////
#define SQ_LAYER_BY_TARGET
#include "CastPointShadowMultiPass.hlsl"
