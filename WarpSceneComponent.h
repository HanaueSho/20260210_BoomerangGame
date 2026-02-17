/*
	WarpSceneComponent.h
	20260217  hanaue sho
*/
#ifndef WARPSCENECOMPONENT_H_
#define WARPSCENECOMPONENT_H_
#include "Component.h"
#include "Manager.h"
#include "GameMainScene.h"
#include "GameStage0Scene.h"
#include "ColliderComponent.h"
#include "InputSystem.h"
#include "FadeSpriteObject.h"

class WarpSceneComponent : public Component
{
public:
	enum class Type
	{
		Stage0,
		Stage1,
	};
private:
	Type m_Type = Type::Stage0;

	bool m_IsFadeChangeScene = false;
public:
	void Init() override {}
	void Update(float gameDt) override
	{
		if (m_IsFadeChangeScene)
		{
			auto* fade = Manager::GetScene()->GetGameObject<FadeSpriteObject>();
			if (fade->EndFadeIn())
			{
				if (m_Type == Type::Stage0)
					Manager::SetScene<GameStage0Scene>();
				//if (m_Type == Type::Stage1)
				//	Manager::SetScene<GameStage1Scene>();
			}
		}
	}

	void OnTriggerEnter(class Collider* me, class Collider* other) {}
	void OnTriggerStay(class Collider* me, class Collider* other) override
	{
			printf("%s\n", other->Owner()->Tag().c_str());
		if (other->Owner()->Tag() == "PlayerHeart")
		{
			if (InputSystem::IsWarpSceneTrigger())
			{
				auto* fade = Manager::GetScene()->GetGameObject<FadeSpriteObject>();
				fade->FadeIn();
				m_IsFadeChangeScene = true;
			}
		}
	}
	void OnTriggerExit(class Collider* me, class Collider* other) {}

};

#endif