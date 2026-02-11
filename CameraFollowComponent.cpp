/*
	CameraFollowComponent.h
	20260211  hanaue sho
	対象を追従するカメラ
	入力で回転もできる
*/
#include "CameraFollowComponent.h"
#include "GameObject.h"
#include "Mouse.h"

void CameraFollowComponent::Update(float dt)
{

	switch (m_State)
	{
	case State::None:
		break;
	case State::Follow:
		Follow();
		break;
	case State::Aim:
		Aim();
		break;
	}



}

void CameraFollowComponent::Follow()
{
	if (!m_pTarget) return;

	// クリックしている間にマウスの移動量を反映 -----
	Vector3 mouseDiff = Vector3(Mouse_GetPositionDiff().x, Mouse_GetPositionDiff().y, 0);
	if (Mouse_IsClick(MS_CLICK_MIDDLE))
	{
		m_YawRadian -= mouseDiff.x * 0.01f;
		m_PitchRadian += mouseDiff.y * 0.01f;
		m_PitchRadian = Vector3::Clamp(m_PitchRadian, -3.1415926535f / 2, 3.1415926535f / 2);
	}

	// ----- 追従 ----- 
	Vector3 targetPos = m_pTarget->Transform()->Position();
	// カメラ位置算出
	float y = sinf(m_PitchRadian);
	float x = cosf(m_YawRadian - 3.1415926535f / 2);
	float z = sinf(m_YawRadian - 3.1415926535f / 2);
	// 距離を一定に保つ
	Vector3 vect = { x, y, z };
	vect.normalize();

	// 本当の目標
	Vector3 targetPosition = targetPos + vect * m_Distance;
	//targetPosition.y += 5.0f; // 補正
	targetPos += m_Offset;

	// 線形補間
	Vector3 position = Owner()->Transform()->Position();
	Vector3 diff = targetPosition - position;
	position += diff * 0.1f;

	Owner()->Transform()->SetPosition(position);

	// 向く
	Owner()->Transform()->LookAt(targetPos);
}

void CameraFollowComponent::Aim()
{
	// 肩越しカメラ
	Vector3 offset = m_Offset; // 調整
	Vector3 targetPosition = m_pTarget->Transform()->WorldMatrix().TransformPoint(offset);
	Vector3 position = Owner()->Transform()->Position();
	Vector3 diff = targetPosition - position;
	position += diff * 0.2f;
	Owner()->Transform()->SetPosition(position);
}
