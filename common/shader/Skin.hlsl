////////////////
//  Skin: the joints of a skinned mesh (Scene::SkinnedMesh): the matrix of each joint (its world
//  matrix by the inverse of its bind matrix: a point of the mesh in the world), at most
//  SKIN_JOINTS_MAX (SkinnedMesh::joints_max: the same). A vertex moves by its 4 joints and their
//  weights (Position3DNormalTangetBinomialUVSkin).
////////////////
#pragma once
#define SKIN_JOINTS_MAX 128

cbuffer Skin
{
	Mat4 skin_joints[SKIN_JOINTS_MAX];
};

// the matrix of a vertex: its joints by their weights
Mat4 skin_matrix(in Vec4 joints, in Vec4 weights)
{
	const int4 id = int4(joints + 0.5);
	return skin_joints[id.x] * weights.x
	     + skin_joints[id.y] * weights.y
	     + skin_joints[id.z] * weights.z
	     + skin_joints[id.w] * weights.w;
}

// a point of the mesh in the world
Vec4 mul_skin(in Vec3 vertex, in Mat4 skin)
{
	return mul(Vec4(vertex, 1.0), skin);
}

// a direction of the mesh in the world (a normal, a tangent: the joints have no scale apart)
Vec3 mul_skin_direction(in Vec3 direction, in Mat4 skin)
{
	return normalize(mul(Vec4(direction, 0.0), skin).xyz);
}
