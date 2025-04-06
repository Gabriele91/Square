// SHADOW UTILS

// Shadow of a spot light
struct SpotShadowCameraStruct
{
	Mat4 m_projection;
	Mat4 m_view;
};
cbuffer SpotShadowCamera
{
	SpotShadowCameraStruct spot_shadow_camera;
}

Vec4 mul_model_spot_light_view_projection(in Vec3 vertex)
{
	Vec4 position = Vec4(vertex, 1.0);
	position = mul(position, transform.m_model);
	position = mul(position, spot_shadow_camera.m_view);
	position = mul(position, spot_shadow_camera.m_projection);
	return position;
}

Vec4 mul_spot_light_view_projection(in Vec4 position)
{
	position = mul(position, spot_shadow_camera.m_view);
	position = mul(position, spot_shadow_camera.m_projection);
	return position;
}

// Shadow of a Point light
struct PointShadowCameraStruct
{
	Mat4 m_projection;
	Mat4 m_view[6];
	Vec4 m_position;
	float m_radius;
};
cbuffer PointShadowCamera
{
	PointShadowCameraStruct point_shadow_camera;
}

Vec4 mul_model_point_light_view_projection(in Vec3 vertex, uint cube_id)
{
	Vec4 position = Vec4(vertex, 1.0);
	position = mul(position, transform.m_model);
	position = mul(position, point_shadow_camera.m_view[cube_map_view_id[cube_id]]);
	position = mul(position, point_shadow_camera.m_projection);
	return position;
}

Vec4 mul_point_light_view_projection(in Vec4 position, uint cube_id)
{
	position = mul(position, point_shadow_camera.m_view[cube_map_view_id[cube_id]]);
	position = mul(position, point_shadow_camera.m_projection);
	return position;
}

// Shadow of a Direction light
#define DIRECTION_SHADOW_CSM_NUMBER_OF_FACES 4
struct DirectionShadowCameraStruct
{
	Mat4 m_projection[DIRECTION_SHADOW_CSM_NUMBER_OF_FACES];
	Mat4 m_view[DIRECTION_SHADOW_CSM_NUMBER_OF_FACES];
	Vec3 m_data[DIRECTION_SHADOW_CSM_NUMBER_OF_FACES];
};

cbuffer DirectionShadowCamera
{
	DirectionShadowCameraStruct direction_shadow_camera;
}

Vec4 mul_model_direction_light_view_projection(in Vec3 vertex, uint id)
{
	Vec4 position = Vec4(vertex, 1.0);
	position = mul(position, transform.m_model);
	position = mul(position, direction_shadow_camera.m_view[id]);
	position = mul(position, direction_shadow_camera.m_projection[id]);
	return position;
}

Vec4 mul_direction_light_view_projection(in Vec4 position, uint id)
{
	position = mul(position, direction_shadow_camera.m_view[id]);
	position = mul(position, direction_shadow_camera.m_projection[id]);
	return position;
}
