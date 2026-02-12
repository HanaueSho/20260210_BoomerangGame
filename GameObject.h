/*
	GameObject.h
	20250813  hanaue sho
	Component志向へ改造（ヘッダ内へ完結）
*/
#ifndef GAMEOBJECT_H_
#define GAMEOBJECT_H_
#include <vector>
#include <memory>
#include <type_traits>
#include <utility>
#include "Component.h"
#include "TransformComponent.h"


class GameObject
{
protected:
	bool m_IsDestroy = false; // 破棄フラグ
	std::vector<std::unique_ptr<Component>> m_Components;
	// --------------------------------------------------
	// タグ
	// --------------------------------------------------
	std::string m_Tag = "None";
	// --------------------------------------------------
	// 物理レイヤー（０～３１想定）
	// --------------------------------------------------
	int m_PhysicsLayer = 0;
public:
	GameObject()
	{
		// 必須：Transform を１つだけ付与（重複付与はしない設計）
		AddComponent<TransformComponent>();
	}
	virtual ~GameObject() = default;

	// ----- ライフサイクル ----- （付与済み全コンポーネントに配信）
	virtual void Init()					{ for (auto& c : m_Components) c->Init(); }
	virtual void Uninit()				{ for (auto& c : m_Components) c->Uninit(); }
	virtual void FixedUpdate(float dt)	{ for (auto& c : m_Components) c->FixedUpdate(dt); }
	virtual void Update(float dt)		{ for (auto& c : m_Components) c->Update(dt); }
	virtual void Draw()					{ for (auto& c : m_Components) c->Draw(); }

	// ----- 破棄フラグ -----
	void RequestDestroy()			noexcept { m_IsDestroy = true; }
	bool IsDestroyRequested() const noexcept { return m_IsDestroy; }

	// ----- コンポネント管理 -----
	// ※ここで Owner 設定と OnAdd を行うよ
	template<class T, class... Args> 
	T* AddComponent(Args&&... args)
	{
		static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");
		auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
		T* raw = ptr.get();
		// friend 指定により GameObject だけが owner を設定可能
		raw->m_pOwner = this;
		m_Components.emplace_back(std::move(ptr));
		raw->OnAdded(); // AddComponent 時に呼ぶ関数
		return raw;
	}

	// 非const版：書き換え可能な T* を返す
	template<class T>
	T* GetComponent() noexcept 
	{
		for (auto& c : m_Components)
			if (auto p = dynamic_cast<T*>(c.get())) return p;
		return nullptr;
	}

	// const版：読み取り専用の const T* を返す
	template<class T>
	const T* GetComponent() const noexcept
	{
		for (auto& c : m_Components)
			if (auto p = dynamic_cast<const T*>(c.get())) return p;
		return nullptr;
	}

	// コンポーネントの取得（ヘルパ）
	template<class F> // callable の F 
	void ForEachComponent(F&& f) // F&& : 転送参照（lvalueもrvalueも受け取れる万能な受け口？？）
	{
		std::vector<Component*> snapshot; 
		snapshot.reserve(m_Components.size());
		for (auto& up : m_Components) if (up) snapshot.push_back(up.get()); // スナップショット（コピー）を取る
		for (Component* c : snapshot) if (c) std::forward<F>(f) (c); //  f(c)... 関数 F を引数 c で呼ぶ （？）
	}

	// Transform は必ず１つ存在（コンストラクタで付与）
	TransformComponent* Transform()			noexcept { return GetComponent<TransformComponent>(); }
	const TransformComponent* Transform() const	noexcept { return GetComponent<TransformComponent>(); }

	// セッターゲッター
	const std::string& Tag() const { return m_Tag; }
	void SetTag(const std::string& tag) { m_Tag = tag; }
	bool CompareTag(const std::string& tag) const { return m_Tag == tag; }
	int PhysicsLayer() const { return m_PhysicsLayer; }
	void SetPhysicsLayer(int layer) { m_PhysicsLayer = layer; }
};

#endif