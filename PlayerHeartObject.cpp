/*
	PlayerHeartObject.cpp
	20260215  haanue sho
*/
#include "PlayerHeartObject.h"
#include "ColliderComponent.h"

void PlayerHeartObject::Init()
{
	// 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0,0,0 });
	tf->SetScale({ 1,1,1 });
	tf->SetEulerAngles({ 0,0,0 });

	// 物理を働かせたいのでコライダーなどを設定
	Collider* coll = AddComponent<Collider>();
	coll->SetCapsule(1.5f, 7.0f);
	coll->SetModeTrigger();

	// タグ
	SetTag("PlayerHeart");
}
