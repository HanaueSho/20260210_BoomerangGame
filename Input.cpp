/*
	Input.cpp
	20260216  hanaue sho
	外部入力をまとめて一意に管理するクラス
*/
#include "Input.h"

std::array<InputGamepad, Input::MAX_PADS> Input::s_Pads = {
	InputGamepad(0),
	InputGamepad(1),
	InputGamepad(2),
	InputGamepad(3),
};

void Input::Init()
{
	// 今後キーボードなどのInitを呼ぶ
}

void Input::Update()
{
	for (auto& pad : s_Pads)
	{
		pad.Update();
	}
}

const InputGamepad& Input::Pad(int playerIndex)
{
	const int index = ClampPadIndex(playerIndex);
	return s_Pads[index];
}
