/*
	CameraComponent.cpp
	20251007  haanue sho
*/
#include "CameraComponent.h"
#include <cmath>

void CameraComponent::Update(float dt)
{
	if (!m_IsMain) return;

	Matrix4x4 view;
	
	view = (m_Mode == Mode::Perspective) ? CalcView3D() : CalcView2D();
	m_CachedView = view;

	if (m_ProjDirty) // 変更があったときだけ処理するよ
	{
		m_CachedProj = (m_Mode == Mode::Perspective) ? CalcProj3D() : CalcProj2D();
		m_ProjDirty = false;
	}
	Vector3 camPos = m_pTransform ? m_pTransform->Position() : Vector3(0, 0, 0);
	//Renderer::SetCamera(view, m_CachedProj, camPos, m_FovY, SCREEN_WIDTH, SCREEN_HEIGHT);
}

Matrix4x4 CameraComponent::CalcView3D()
{
	const Vector3 pos	  = m_pTransform->Position();
	const Vector3 forward = m_pTransform->Forward().normalized();
	const Vector3 up	  = m_pTransform->Up().normalized();

	Vector3 right = Vector3::Cross(up, forward).normalized();
	Vector3 newUp = Vector3::Cross(forward, right);

	// ほぼ平行対策
	if (std::fabs(Vector3::Dot(forward, newUp)) > 0.999f)
		newUp = (std::fabs(forward.y) > 0.999f) ? Vector3(1, 0, 0) : Vector3(0, 1, 0);

	// 行ベクトル版(DirectX)に合わせる
	Matrix4x4 view;
	view.identity();
	// 回転
	view.m[0][0] = right.x; view.m[0][1] = newUp.x; view.m[0][2] = forward.x;
	view.m[1][0] = right.y; view.m[1][1] = newUp.y; view.m[1][2] = forward.y;
	view.m[2][0] = right.z; view.m[2][1] = newUp.z; view.m[2][2] = forward.z;

	// 平行移動
	view.m[3][0] = -Vector3::Dot(right,	  pos);
	view.m[3][1] = -Vector3::Dot(newUp,   pos);
	view.m[3][2] = -Vector3::Dot(forward, pos);

	return view;
}
Matrix4x4 CameraComponent::CalcProj3D()
{
	float yScale = 1.0f / std::tanf(m_FovY * 0.5f);
	float xScale = yScale / m_Aspect;
	float zf = m_FarZ;
	float zn = m_NearZ;

	Matrix4x4 proj;
	proj.identity();

	proj.m[0][0] = xScale;
	proj.m[1][1] = yScale;
	proj.m[2][2] = zf / (zf - zn);
	proj.m[2][3] = 1.0f;
	proj.m[3][2] = (-zn * zf) / (zf - zn);
	proj.m[3][3] = 0.0f;

	return proj;
}
Matrix4x4 CameraComponent::CalcView2D()
{
	Matrix4x4 view; 
	view.identity();

	if (!m_UseTransformIn2D || !m_pTransform) return view; // 固定カメラ（原点）

	// 2Dパン：カメラ位置 C を使って View = Translate(-C)
	const Vector3 pos = m_pTransform->Position();
	view.m[3][0] = -pos.x;
	view.m[3][1] = -pos.y;
	view.m[3][2] = -pos.z;

	// ※※※※※　もしかしたらここでTransformのIsDirtyを消費する必要がある？ない？　※※※※※
	return view;
}
Matrix4x4 CameraComponent::CalcProj2D()
{
	Matrix4x4 proj;
	proj.identity();

	float l = m_OrthoLeft;
	float r = m_OrthoRight;
	float b = m_OrthoBottom;
	float t = m_OrthoTop;
	float n = m_OrthoNearZ;
	float f = m_OrthoFarZ;

	proj.m[0][0] = 2.0f / (r - l);
	proj.m[1][1] = 2.0f / (t - b); // top = 0, bottom = H のとき負（Y反転）
	proj.m[2][2] = 1.0f / (f - n);
	proj.m[3][0] = (l + r) / (l - r);
	proj.m[3][1] = (b + t) / (b - t);
	proj.m[3][2] = n / (n - f);

	return proj;
}
