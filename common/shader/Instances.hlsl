////////////////
//  Instances: the instances of an instanced draw (Scene::InstancedMesh): the matrix of each one in
//  the space of its actor, read by SV_InstanceID; the transform of the actor after it. At most
//  INSTANCES_MAX a draw (InstancedMesh::instances_max: the same)
////////////////
#pragma once
#include <Transform>
#define INSTANCES_MAX 192

cbuffer Instances
{
	Mat4 instances_model[INSTANCES_MAX];
};

// a point of an instance in the world: its matrix, then the actor's
Vec4 mul_instance_model(in Vec3 vertex, uint id)
{
	return mul(mul(Vec4(vertex, 1.0), instances_model[id]), transform.m_model);
}

// a direction of an instance (a normal, a tangent) in the space of its actor (its scale out)
Vec3 mul_instance_direction(in Vec3 direction, uint id)
{
	return normalize(mul(Vec4(direction, 0.0), instances_model[id]).xyz);
}
