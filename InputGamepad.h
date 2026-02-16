/*
	InputGamepad.h
	20260216  hanaue sho
*/
#ifndef INPUTGAMEPAD_H_
#define INPUTGAMEPAD_H_
#include <windows.h>
#include <Xinput.h>
#include <cstdint>
#include <algorithm>
#include <cmath>

#pragma comment(lib, "xinput.lib")

enum class PadButton : WORD
{
	A = XINPUT_GAMEPAD_A,
	B = XINPUT_GAMEPAD_B,
	X = XINPUT_GAMEPAD_X,
	Y = XINPUT_GAMEPAD_Y,

	LB = XINPUT_GAMEPAD_LEFT_SHOULDER,
	RB = XINPUT_GAMEPAD_RIGHT_SHOULDER,

	Back = XINPUT_GAMEPAD_BACK,
	START = XINPUT_GAMEPAD_START,

	LStick = XINPUT_GAMEPAD_LEFT_THUMB,
	RStick = XINPUT_GAMEPAD_RIGHT_THUMB,

	Up	  = XINPUT_GAMEPAD_DPAD_UP,
	Down  = XINPUT_GAMEPAD_DPAD_DOWN,
	Left  = XINPUT_GAMEPAD_DPAD_LEFT,
	Right = XINPUT_GAMEPAD_DPAD_RIGHT,
};

class InputGamepad
{
private:
	int m_Index = 0;
	bool m_Connected = false;

	XINPUT_STATE m_Now{};
	XINPUT_STATE m_Prev{};

	// 解析済み値
	float m_Lt = 0.0f, m_Rt = 0.0f;
	float m_Lx = 0.0f, m_Ly = 0.0f;
	float m_Rx = 0.0f, m_Ry = 0.0f;

	// 解析済み（前フレーム）値：トリガ閾値跨ぎ判定用
	float m_LtPrev = 0.0f, m_RtPrev = 0.0f;

	SHORT m_DeadL = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;  // 7849
	SHORT m_DeadR = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE; // 8689
public:
	// 0~3
	explicit InputGamepad(int playerIndex = 0) : m_Index(playerIndex) {}
	void SetPlayerIndex(int index) { m_Index = index; }

	void Update();

	// 接続状態
	bool IsConnected() const { return m_Connected; }

	// ----- ボタン入力 -----
	bool IsDown(WORD button) const;
	bool IsUp(WORD button) const { return !IsDown(button); }
	bool IsPressed(WORD button) const;  // 押した瞬間
	bool IsReleased(WORD button) const; // 離した瞬間
	// オーバーロード
	bool IsDown(PadButton b)	 const { return IsDown(static_cast<WORD>(b)); }
	bool IsUp(PadButton b)		 const { return !IsDown(static_cast<WORD>(b));}
	bool IsPressed(PadButton b)  const { return IsPressed(static_cast<WORD>(b)); }
	bool IsReleased(PadButton b) const { return IsReleased(static_cast<WORD>(b)); }

	// ----- トリガー(0..1) -----
	float LT() const { return m_Lt; }
	float RT() const { return m_Rt; }

	bool LtDown(float threshold = 0.5f) const { return m_Lt >= threshold; }
	bool RtDown(float threshold = 0.5f) const { return m_Rt >= threshold; }
	bool LtPressed(float threshold = 0.5f) const;
	bool RtPressed(float threshold = 0.5f) const;
	bool LtReleased(float threshold = 0.5f) const;
	bool RtReleased(float threshold = 0.5f) const;

	// ----- スティック(-1..1) -----
	float LX() const { return m_Lx; }
	float LY() const { return m_Ly; }
	float RX() const { return m_Rx; }
	float RY() const { return m_Ry; }

	// デッドゾーン設定（0..32767推奨）
	void SetDeadZoneLeft(SHORT dz)  { m_DeadL = std::clamp<SHORT>(dz, 0, 32767); }
	void SetDeadZoneRight(SHORT dz) { m_DeadR = std::clamp<SHORT>(dz, 0, 32767); }

	// ----- 振動 -----
	void SetVibration(float leftMotor01, float rightMotor01);
	void StopVibration() { SetVibration(0.0f, 0.0f); }

	// 生データが欲しいとき
	const XINPUT_STATE& StateNow() const { return m_Now; }
	const XINPUT_STATE& StatePrev() const { return m_Prev; }

public:
	static float NormalizeTrigger(BYTE v); // 0..255 -> 0..1
	static void  NormalizeStickCircular(SHORT rawX, SHORT rawY, SHORT deadZone, float& outX, float& outY);
	static float Clamp01(float v) { return std::clamp(v, 0.0f, 1.0f); }
	static float ClampN1P1(float v) { return std::clamp(v, -1.0f, 1.0f); }
};

#endif