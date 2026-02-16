/*
	InputGamepad.cpp
	20260216  hanaue sho
*/
#include "InputGamepad.h"

void InputGamepad::Update()
{
	// 前フレーム保持
	m_Prev = m_Now;

	// 閾値跨ぎ用
	m_LtPrev = m_Lt;
	m_RtPrev = m_Rt;

	ZeroMemory(&m_Now, sizeof(m_Now));

	const DWORD res = XInputGetState(static_cast<DWORD>(m_Index), &m_Now);
	m_Connected = (res == ERROR_SUCCESS);

	if (!m_Connected)
	{
		// 切断状態：状態を０クリア
		ZeroMemory(&m_Now, sizeof(m_Now));
		m_Lt = m_Rt = 0.0f;
		m_Lx = m_Ly = m_Rx = m_Ry = 0.0f;
		return;
	}

	// トリガ
	m_Lt = NormalizeTrigger(m_Now.Gamepad.bLeftTrigger);
	m_Rt = NormalizeTrigger(m_Now.Gamepad.bRightTrigger);

	// スティック（円形デッドゾーン）
	NormalizeStickCircular(m_Now.Gamepad.sThumbLX, m_Now.Gamepad.sThumbLY, m_DeadL, m_Lx, m_Ly);
	NormalizeStickCircular(m_Now.Gamepad.sThumbRX, m_Now.Gamepad.sThumbRY, m_DeadR, m_Rx, m_Ry);
}

bool InputGamepad::IsDown(WORD button) const
{
	if (!m_Connected) return false;
	return (m_Now.Gamepad.wButtons & button) != 0;
}

bool InputGamepad::IsPressed(WORD button) const
{
	if (!m_Connected) return false;
	const bool now  = (m_Now.Gamepad.wButtons  & button) != 0;
	const bool prev = (m_Prev.Gamepad.wButtons & button) != 0;
	return now && !prev;
}

bool InputGamepad::IsReleased(WORD button) const
{
	if (!m_Connected) return false;
	const bool now = (m_Now.Gamepad.wButtons & button) != 0;
	const bool prev = (m_Prev.Gamepad.wButtons & button) != 0;
	return !now && prev;
}

bool InputGamepad::LtPressed(float threshold) const
{
	if (!m_Connected) return false;
	return (m_Lt >= threshold) && (m_LtPrev < threshold);
}

bool InputGamepad::RtPressed(float threshold) const
{
	if (!m_Connected) return false;
	return (m_Rt >= threshold) && (m_RtPrev < threshold);
}

void InputGamepad::SetVibration(float leftMotor01, float rightMotor01)
{
	if (!m_Connected) return;

	leftMotor01  = Clamp01(leftMotor01);
	rightMotor01 = Clamp01(rightMotor01);
	
	XINPUT_VIBRATION vib{};
	vib.wLeftMotorSpeed  = static_cast<WORD>(leftMotor01  * 65535.0f);
	vib.wRightMotorSpeed = static_cast<WORD>(rightMotor01 * 65535.0f);
	XInputSetState(static_cast<DWORD>(m_Index), &vib);
}

float InputGamepad::NormalizeTrigger(BYTE v)
{
	return static_cast<float>(v) / 255.0f;
}

void InputGamepad::NormalizeStickCircular(SHORT rawX, SHORT rawY, SHORT deadZone, float& outX, float& outY)
{
	// rawX/rawY: -32768..32767
	const float x = static_cast<float>(rawX);
	const float y = static_cast<float>(rawY);

	const float mag = std::sqrt(x * x + y * y);

	if (mag <= static_cast<float>(deadZone))
	{
		outX = 0.0f;
		outY = 0.0f;
		return;
	}

	// deadZone を再スケール
	const float maxMag = 32767.0f;
	const float legalMag = std::min(mag, maxMag);

	const float normalMag = (legalMag - deadZone) / (maxMag - deadZone);
	const float nx = x / mag;
	const float ny = y / mag;

	outX = ClampN1P1(nx * normalMag);
	outY = ClampN1P1(ny * normalMag);
	return;
}
