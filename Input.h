/*
	Input.h
	20260216  hanaue sho
	外部入力をまとめて一意に管理するクラス
	【具体例】
	Input::Pad(0).IsDown(XINPUT_BUTTON_A)
*/
#ifndef INPUT_H_
#define INPUT_H_
#include <array>
#include <cassert>
#include "InputGamepad.h"

class Input
{
private:
	static constexpr int MAX_PADS = 4;
	static std::array<InputGamepad, MAX_PADS> s_Pads;

	static int ClampPadIndex(int playerIndex)
	{
		if (playerIndex < 0) return 0;
		if (playerIndex >= MAX_PADS) return MAX_PADS - 1;
		return playerIndex;
	}

public:
	// 初期化
	static void Init();

	// 毎フレーム１度だけ呼ぶ（ここで一元管理）
	static void Update();

	// playerIndex: 0..3
	static const InputGamepad& Pad(int playerIndex);
};

#endif