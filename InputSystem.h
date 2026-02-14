/*
	InputSystem.h
	20260210  hanaue sho
*/
#ifndef INPUTSYSTEM_H_
#define INPUTSYSTEM_H_
#include "Keyboard.h"
#include "Mouse.h"
#include "Vector3.h"

class InputSystem
{
private:
	static Vector3 m_Input;
public:
	static bool IsMoveDown()
	{
		return Keyboard_IsKeyDown(KK_A) ||
			   Keyboard_IsKeyDown(KK_D) ||
			   Keyboard_IsKeyDown(KK_W) ||
			   Keyboard_IsKeyDown(KK_S) ; 
	}
	static bool IsJumpDown()
	{
		return Keyboard_IsKeyDown(KK_SPACE);
	}
	static bool IsJumpDownTrigger()
	{
		return Keyboard_IsKeyDownTrigger(KK_SPACE);
	}
	static bool IsToAimDown()
	{
		return Mouse_IsClick(MS_CLICK_RIGHT);
	}
	static bool IsAimDownTrigger()
	{
		return Mouse_IsClick(MS_CLICK_LEFT);
	}
	static bool IsThrowDown()
	{
		return Mouse_IsClick(MS_CLICK_LEFT);
	}
	static bool IsThrowUp()
	{
		return Mouse_IsClickUp(MS_CLICK_RIGHT);
	}
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
		return m_Input;
	}
};

#endif