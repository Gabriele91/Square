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
Vec4 mul_spot_light_view_projection(Vec4 position)
{
	position = mul(position, spot_shadow_camera.m_view);
	position = mul(position, spot_shadow_camera.m_projection);
	return position;
}

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

Vec4 mul_point_light_view_projection(Vec4 position, uint cube_id)
{
	position = mul(position, point_shadow_camera.m_view[cube_map_view_id[cube_id]]);
	position = mul(position, point_shadow_camera.m_projection);
	return position;
}