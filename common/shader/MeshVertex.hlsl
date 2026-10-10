////////////////
//  MeshVertex: the vertex of a mesh and its point in the world, the same for a static mesh and a
//  skinned one (the skinned variant of a technique: SQ_SKINNED, its vertex has its joints: Skin.hlsl).
//  The shaders that only need where a vertex is (the shadows) take MeshVertex and
//  mesh_world_position.
////////////////
#pragma once
#include <Vertex>
#include <Transform>
#ifdef SQ_SKINNED
#include <Skin>
#define MeshVertex Position3DNormalTangetBinomialUVSkin

// its point in the world: by its joints
Vec4 mesh_world_position(in MeshVertex input)
{
	return mul_skin(input.m_position, skin_matrix(input.m_joints, input.m_weights));
}
#else
#define MeshVertex Position3DNormalTangetBinomialUV

// its point in the world: by the model matrix
Vec4 mesh_world_position(in MeshVertex input)
{
	return mul(Vec4(input.m_position, 1.0), transform.m_model);
}
#endif
