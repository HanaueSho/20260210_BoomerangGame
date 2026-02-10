/*
	Result.h
	20250625  hanaue sho
*/
#ifndef RESULT_H_
#define RESULT_H_
#include "scene.h"


class Result : public Scene
{
private:

public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;
};





#endif