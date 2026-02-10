/*
	CameraMove.h
	20250820  hanaue sho
*/
#include "CameraMove.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Vector3.h"

void CameraMove::Update(float dt)
{
	if (!m_pTransform) return;

	// クリックしている間にマウスの移動量を反映 -----
	Vector3 mouseDiff = Vector3(Mouse_GetPositionDiff().x, Mouse_GetPositionDiff().y, 0);
	Vector3 right	  = m_pTransform->Right();
	Vector3 up		  = m_pTransform->Up();
	Vector3 forward   = m_pTransform->Forward();

	// 右クリックで回転 -----
	if (Mouse_IsClick(MS_CLICK_RIGHT))
	{
		m_pTransform->RotateAxis({ 0, 1, 0 }, mouseDiff.x * 0.005f);
		m_pTransform->RotateAxis(right, mouseDiff.y * 0.005f);
	}
	// 中クリックで移動 -----
	if (Mouse_IsClick(MS_CLICK_MIDDLE))
	{
		Vector3 position = m_pTransform->Position();
		position += right * mouseDiff.x * -0.01f;
		position += up * mouseDiff.y * 0.01f;
		m_pTransform->SetPosition(position);
	}
	// ホイールで前後移動 -----
	if (Mouse_IsWheelScroll())
	{
		Vector3 scroll = forward * Mouse_GetWheelScroll() * 0.5f; // 距離調整
		Vector3 position = m_pTransform->Position();
		position += scroll;
		m_pTransform->SetPosition(position);
	}
}
