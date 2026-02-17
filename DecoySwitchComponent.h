/*
	DecoySwitchComponent.h
	20260217  hanaue sho
*/
#ifndef DECOYSWITCHCOMPONENT_H_
#define DECOYSWITCHCOMPONENT_H_
#include "Component.h"
#include "Manager.h"
#include "GameMainScene.h"
#include "ColliderComponent.h"
#include "InputSystem.h"
#include "FadeSpriteObject.h"

class DecoySwitchComponent : public Component
{
private:
public:
	void Init() override {}

	void OnTriggerEnter(class Collider* me, class Collider* other) {}
	void OnTriggerStay(class Collider* me, class Collider* other) override
	{
		if (other->Owner()->Tag() == "PlayerHeart")
		{
			if (InputSystem::IsWarpSceneTrigger())
			{
				dynamic_cast<GameMainScene*>(Manager::GetScene())->ResetDecoies();
			}
		}
	}
	void OnTriggerExit(class Collider* me, class Collider* other) {}

};

#endif