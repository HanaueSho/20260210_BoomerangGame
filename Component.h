/*
	Component.h
	20250813  hanaue sho
	学校のやつを改造
*/
#ifndef COMPONENT_H_
#define COMPONENT_H_

class GameObject; // 前方宣言

class Component
{
protected:
	GameObject* m_pOwner = nullptr;

public:
	Component() = default;
	virtual ~Component() = default;

	// 所有者アクセス（読み取り専用API）
	GameObject*			Owner()		  noexcept  { return m_pOwner; }
	const GameObject*	Owner() const noexcept	{ return m_pOwner; }

	// ----- ライフサイクル -----
	virtual void OnAdded() {}		// AddComponent時
	virtual void Init() {}			// GameObject.Init()時
	virtual void Uninit() {}		// GameObject破棄時

	// ----- 更新 -----
	virtual void FixedUpdate(float dt) {}	// 固定更新（物理演算系）
	virtual void Update(float dt) {}		// 通常更新
	virtual void Draw() {}					// 描画

	// ----- ディスパッチ -----
	virtual void OnTriggerEnter(class Collider* me, class Collider* other) {}
	virtual void OnTriggerStay (class Collider* me, class Collider* other) {}
	virtual void OnTriggerExit (class Collider* me, class Collider* other) {}

	virtual void OnCollisionEnter(class Collider* me, class Collider* other) {}
	virtual void OnCollisionStay (class Collider* me, class Collider* other) {}
	virtual void OnCollisionExit (class Collider* me, class Collider* other) {}


	// コピー、ムーブは原則禁止（所有者とライフサイクルが絡むため）
	Component(const Component&) = delete;
	Component& operator =(const Component&) = delete;
	Component(Component&&) = delete;
	Component& operator =(const Component&&) = delete;

	// owner を設定できるのは GameObject だけに限定
	friend class GameObject;
};

#endif