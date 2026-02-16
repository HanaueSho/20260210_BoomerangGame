/*
	CameraFollowComponent.h
	20260211  hanaue sho
	対象を追従するカメラ
	入力で回転もできる
*/
#include "CameraFollowComponent.h"
#include "GameObject.h"
#include "Mouse.h"
#include "Input.h"
#include "InputSystem.h"

namespace
{
	Vector3 ForwardFromYawPitch(float yaw, float pitch)
	{
		const float cp = cosf(pitch);
		const float sp = sinf(pitch);
		const float cy = cosf(yaw);
		const float sy = sinf(yaw);

		Vector3 forward;
		forward.x = cp * cy;
		forward.y = sp;
		forward.z = cp * sy;
		forward.normalize();
		return forward;
	}

}

void CameraFollowComponent::Update(float dt)
{
	switch (m_State)
	{
	case State::None:
		break;
	case State::Follow:
		Follow(dt);
		AddShake(dt);
		break;
	case State::Aim:
		Aim(dt);
		AddShake(dt);
		break;
	}
}

void CameraFollowComponent::ChangeState(State newState)
{
	// 終了処理
	switch (m_State)
	{
	case State::None:
		break;
	case State::Follow:
		break;
	case State::Aim:
		// カメラ角度の調整

		break;
	}

	m_State = newState;
	// 初期化処理
	switch (m_State)
	{
	case State::None:
		break;
	case State::Follow:
		break;
	case State::Aim:
		break;
	}
}

void CameraFollowComponent::Follow(float dt)
{
	if (!m_pTarget) return;

	// クリックしている間にマウスの移動量を反映 -----
	Vector3 mouseDiff = Vector3(Mouse_GetPositionDiff().x, Mouse_GetPositionDiff().y, 0);
	if (Input::Pad(0).IsConnected()) // コントローラー優先
	{
		m_YawRadian   += Input::Pad(0).RX() * dt * -3.0f;
		m_PitchRadian += Input::Pad(0).RY() * dt * -3.0f;
		m_PitchRadian = Vector3::Clamp(m_PitchRadian, -3.1415926535f / 2, 3.1415926535f / 2);
	}
	else if (Mouse_IsClick(MS_CLICK_MIDDLE))
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

	const float k = 6.32f * 2;
	float alpha = 1.0f - expf(-k * dt);
	if (alpha < 0.0f) alpha = 0.0f;
	if (alpha > 1.0f) alpha = 1.0f;

	position += diff * alpha;
	Owner()->Transform()->SetPosition(position);

	// ----- カメラ回転 -----
	TransformComponent* camTf = Owner()->Transform();
	Vector3 camPos = camTf->Position();

	// 注視方向
	Vector3 fwd = targetPos - camPos;
	if (fwd.lengthSq() > 1e-8f)
	{
		fwd.normalize();
		//	ワールド上方向
		const Vector3 up = { 0, 1, 0 };
		// 目標回転
		Quaternion qTarget = Quaternion::LookRotation(fwd, up);
		// 現在回転
		Quaternion qCur = camTf->Rotation();

		// dt 依存の補間
		float t = 1.0f - expf(-m_LookK * dt);
		if (t < 0.0f) t = 0.0f;
		if (t > 1.0f) t = 1.0f;
		Quaternion qNew = Quaternion::Slerp(qCur, qTarget, t);

		camTf->SetRotation(qNew);
	}
}

void CameraFollowComponent::Aim(float dt)
{
	// 肩越しカメラ
	Vector3 offset = m_Offset; // 調整
	Vector3 targetPosition = m_pTarget->Transform()->WorldMatrix().TransformPoint(offset);
	Vector3 position = Owner()->Transform()->Position();
	Vector3 diff = targetPosition - position;

	const float k = 10.0f; // Aimは追従強めでもOK
	float alpha = 1.0f - expf(-k * dt);
	if (alpha < 0.0f) alpha = 0.0f;
	if (alpha > 1.0f) alpha = 1.0f;

	position += diff * alpha;
	Owner()->Transform()->SetPosition(position);

	if (Input::Pad(0).IsConnected()) // コントローラー優先
	{
		float yawRadianDelta   = Input::Pad(0).RX() * dt * -2.0f;
		float pitchRadianDelta = Input::Pad(0).RY() * dt * -2.0f;

		Owner()->Transform()->RotateAxis({ 0, 1, 0 }, -yawRadianDelta);
		Vector3 right = Owner()->Transform()->Right();
		Owner()->Transform()->RotateAxis(right, pitchRadianDelta);

		// 更新
		m_YawRadian   += yawRadianDelta;
		m_PitchRadian += pitchRadianDelta;
		m_PitchRadian = Vector3::Clamp(m_PitchRadian, -3.1415926535f / 2, 3.1415926535f / 2);
	}
}

void CameraFollowComponent::AddShake(float dt)
{
	if (!m_IsShake) return;

	m_ShakeTimer += dt;
	if (m_ShakeTimer > m_ShakeTime)
	{
		m_IsShake = false;
		m_ShakeOffset = { 0, 0, 0 };
		m_ShakeTimer = 0.0f;
	}

	Vector3 shake = m_ShakeValue * m_ShakeScale;
	m_ShakeOffset = shake * m_ShakeSign;

	Vector3 position = Owner()->Transform()->Position();
	position += m_ShakeOffset;
	Owner()->Transform()->SetPosition(position);

	// 上下反転
	m_ShakeSign = -m_ShakeSign;
}
