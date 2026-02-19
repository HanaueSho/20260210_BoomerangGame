/*
	Title.h
	20250625  hanaue sho
*/
#ifndef TITLE_H_
#define TITLE_H_
#include "scene.h"

class GameObject;

class Title : public Scene
{
private:
	bool m_IsFadeChangeScene = false;

public:
	void Init() override;
	void Uninit() override;
	void Update(float gameDt, float realDt) override;


};





#endif