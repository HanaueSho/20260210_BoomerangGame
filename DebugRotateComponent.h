/*
	RotateComponent.h
*/
#pragma once
#include "Component.h"
#include "GameObject.h"
#include "RigidbodyComponent.h"

class DebugRotateComponent : public Component
{
private:
	GameObject* pObject = nullptr;
public:
	void Update(float dt) 
	{
		if (!pObject) return;
		Vector3 angular = pObject->GetComponent<Rigidbody>()->AngularVelocity();
		angular.x *= 10.0f;
		angular.y *= 10.0f;
		angular.z *= 10.0f;
		//Owner()->GetComponent<Rigidbody>()->SetAngularVelocity(angular);
	}
	void SetObject(GameObject* p) { pObject = p; }
};




