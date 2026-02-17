/*
	DamageComponent.h
	20260217  hanaue sho
*/
#ifndef DAMAGECOMPONENT_H_
#define DAMAGECOMPONENT_H_
#include "Component.h"
#include "GameObject.h"
#include "ColliderComponent.h"
#include "PlayerHeartObject.h"
#include "PlayerStateManagerComponent.h"

class DamageComponent : public Component
{
private:
	bool m_IsActive = true;

public:
	void Init() override {}

	void SetActive(bool b) { m_IsActive = b; }

	void OnTriggerEnter(class Collider* me, class Collider* other) override
	{
		if (!m_IsActive) return;

		if (other->Owner()->Tag() == "PlayerHeart") // プレイヤーハートに当たったら
		{
			//printf("プレイヤーに当たったよ\n");
			// ダメージ処理へ
			PlayerHeartObject* heart = dynamic_cast<PlayerHeartObject*>(other->Owner());
			PlayerStateManagerComponent* state = heart->GetOwnerPlayer()->GetComponent<PlayerStateManagerComponent>();
			state->ChangeStateDamage();
		}
	}

};

#endif