/*
	InputSystem.h
	20260210  hanaue sho
*/
#ifndef INPUTSYSTEM_H_
#define INPUTSYSTEM_H_
#include "Keyboard.h"
#include "Mouse.h"
#include "Input.h"
#include "Vector3.h"

class InputSystem
{
private:
	static Vector3 m_Input;
public:
	// 移動キー入力
	static bool IsMoveDown()
	{
		return Keyboard_IsKeyDown(KK_A)  ||
			   Keyboard_IsKeyDown(KK_D)  ||
			   Keyboard_IsKeyDown(KK_W)  ||
			   Keyboard_IsKeyDown(KK_S)  ||
			  Input::Pad(0).LX() != 0.0f ||
			  Input::Pad(0).LY() != 0.0f; 
	}
	// ジャンプキー入力
	static bool IsJumpDown()
	{
		return Keyboard_IsKeyDown(KK_SPACE) || Input::Pad(0).IsDown(PadButton::A);
	}
	// ジャンプキー入力された瞬間
	static bool IsJumpDownTrigger()
	{
		return Keyboard_IsKeyDownTrigger(KK_SPACE) || Input::Pad(0).IsPressed(PadButton::A);
	}
	// エイム開始キー入力
	static bool IsToAimDown()
	{
		return Mouse_IsClick(MS_CLICK_RIGHT) || Input::Pad(0).RtDown();
	}
	// ターゲットキー入力された瞬間
	static bool IsTargetDownTrigger()
	{
		return Mouse_IsClickTrigger(MS_CLICK_LEFT) || Input::Pad(0).LtPressed();
	}
	// 投擲キー入力
	static bool IsThrowDown()
	{
		return Mouse_IsClick(MS_CLICK_LEFT) || Input::Pad(0).LtDown();
	}
	// 投擲キー入力が終わった瞬間
	static bool IsThrowUp()
	{
		return Mouse_IsClickUp(MS_CLICK_RIGHT) || Input::Pad(0).RtReleased();
	}
	// キックキー入力
	static bool IsKickTrigger()
	{
		return Keyboard_IsKeyDownTrigger(KK_E) || Input::Pad(0).IsPressed(PadButton::B);
	}
	// プッシュキー入力
	static bool IsPushTrigger()
	{
		return Keyboard_IsKeyDownTrigger(KK_Q) || Input::Pad(0).IsPressed(PadButton::X);
	}
	// ワープシーンキー入力
	static bool IsWarpSceneTrigger()
	{
		return Keyboard_IsKeyDownTrigger(KK_E) || Input::Pad(0).IsPressed(PadButton::Y);
	}

	// 移動キーの入力値取得
	static const Vector3& GetInputMove()
	{
		m_Input = { 0, 0, 0 };
		if (Keyboard_IsKeyDown(KK_A))
			m_Input.x += -1;
		if (Keyboard_IsKeyDown(KK_D))
			m_Input.x += 1;
		if (Keyboard_IsKeyDown(KK_W))
			m_Input.z += 1;
		if (Keyboard_IsKeyDown(KK_S))
			m_Input.z += -1;

		if (Input::Pad(0).IsConnected())
		{
			m_Input.x = Input::Pad(0).LX();
			m_Input.z = Input::Pad(0).LY();
		}

		return m_Input;
	}
	// カメラ入力値の取得
	static Vector3 GetInputMoveCamera()
	{
		Vector3 out = {0.0f, 0.0f, 0.0f};
		out.x = Input::Pad(0).RX();
		out.y = Input::Pad(0).RY();
		return out;
	}
	// AnyKey
	static bool AnyKeyDown()
	{
		return
			Input::Pad(0).IsDown(PadButton::A) ||
			Input::Pad(0).IsDown(PadButton::B) ||
			Input::Pad(0).IsDown(PadButton::X) ||
			Input::Pad(0).IsDown(PadButton::Y) ||
			Input::Pad(0).IsDown(PadButton::LB) ||
			Input::Pad(0).IsDown(PadButton::RB) ||
			Input::Pad(0).LtDown() ||
			Input::Pad(0).RtDown();
	}

};

#endif