/*
	Title.h
	20250625  hanaue sho
*/
#ifndef TITLE_H_
#define TITLE_H_
#include "scene.h"


class Title : public Scene
{
private:
public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;
};





#endif