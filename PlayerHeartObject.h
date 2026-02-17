/*
	PlayerHeartObject.h
	20260215  haanue sho
*/
#ifndef PLAYERHEARTOBJECT_H_
#define PLAYERHEARTOBJECT_H_
#include "GameObject.h"

class PlayerHeartObject : public GameObject
{
private:
	GameObject* m_pOwnerPlayer = nullptr;

public:
	void Init() override;

	void SetOwnerPlayer(GameObject* player) { m_pOwnerPlayer = player; }
	GameObject* GetOwnerPlayer() { return m_pOwnerPlayer; }

};

#endif