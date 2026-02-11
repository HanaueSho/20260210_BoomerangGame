/*
	CameraFollowComponent.h
	20260211  hanaue sho
	‘ÎÛ‚ð’Ç]‚·‚éƒJƒƒ‰
	“ü—Í‚Å‰ñ“]‚à‚Å‚«‚é
*/
#include "CameraFollowComponent.h"
#include "GameObject.h"
#include "Mouse.h"

void CameraFollowComponent::Update(float dt)
{
	if (!m_IsFollow || !m_pTarget) return;


	// ƒNƒŠƒbƒN‚µ‚Ä‚¢‚éŠÔ‚Éƒ}ƒEƒX‚ÌˆÚ“®—Ê‚ð”½‰f -----
	Vector3 mouseDiff = Vector3(Mouse_GetPositionDiff().x, Mouse_GetPositionDiff().y, 0);

	if (Mouse_IsClick(MS_CLICK_RIGHT))
	{
		m_YawRadian   -= mouseDiff.x * 0.01f;
		m_PitchRadian += mouseDiff.y * 0.01f;
		m_PitchRadian = Vector3::Clamp(m_PitchRadian, -3.1415926535f / 2, 3.1415926535f / 2);
	}


	// ----- ’Ç] ----- 
	Vector3 targetPos = m_pTarget->Transform()->Position();
	// ƒJƒƒ‰ˆÊ’uŽZo
	float y = sinf(m_PitchRadian);
	float x = cosf(m_YawRadian - 3.1415926535f/2);
	float z = sinf(m_YawRadian - 3.1415926535f/2);
	// ‹——£‚ðˆê’è‚É•Û‚Â
	Vector3 vect = {x, y, z};
	vect.normalize();
	Vector3 position = targetPos + vect * m_Distance;
	position.y += 5.0f;
	Owner()->Transform()->SetPosition(position);

	// Œü‚­
	Owner()->Transform()->LookAt(targetPos);


}
