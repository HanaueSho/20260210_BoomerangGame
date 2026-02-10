/*
	Playroom.h
	20260202  hanaue sho
*/
#ifndef PLAYROOMSCENE_H_
#define PLAYROOMSCENE_H_
#include "scene.h"

class PlayroomScene : public Scene
{
private:

public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;
	void Draw() override;

	void CreateSection_0(); // プレイヤーと軽いオブジェクトなど（０時）
	void CreateSection_1(); // 坂道（１時）
	void CreateSection_2(); // 回転バー（２時）
	void CreateSection_3(); // メッシュフィールド（９時）
	void CreateSection_4(); // こま（４時）
	void CreateSection_5(); // 跳ねる床（５時）

};

#endif