/*
	LightComponent.h
	20251104  hanaue sho
*/
#ifndef LIGHTCOMPONENT_H_
#define LIGHTCOMPONENT_H_
#include "Component.h"
#include "Vector3.h"
#include "TransformComponent.h"
#include "GameObject.h"
#include "Manager.h"
#include "Scene.h"
#include "LightManager.h"

class LightComponent : public Component
{
private:
	// ==================================================
	// ----- 構成要素 -----
	// ==================================================
	// --------------------------------------------------
	// 拡散光色
	// --------------------------------------------------
	Vector3	m_Diffuse = Vector3(1.0f, 1.0f, 1.0f);

	// --------------------------------------------------
	// 有効距離
	// --------------------------------------------------
	float m_Range = 10;

	// --------------------------------------------------
	// 強度
	// --------------------------------------------------
	float m_Intensity = 1.0f;

	// --------------------------------------------------
	// 仕様中
	// --------------------------------------------------
	bool m_Enable = true;
	
	// --------------------------------------------------
	// ダーティ
	// --------------------------------------------------
	bool m_Dirty = false;

	// --------------------------------------------------
	// Transform 参照
	// --------------------------------------------------
	TransformComponent* m_pTransform = nullptr;
public:
	// ==================================================
	// ----- セッターゲッター-----
	// ==================================================
	const Vector3& Diffuse() const { return m_Diffuse; }
	float Range()			 const { return m_Range; }
	float Intensity()		 const { return m_Intensity; }
	bool  IsDirty()			 const { return m_Dirty; }
	bool  Enable()			 const { return m_Enable; }
	void SetDiffuse  (const Vector3& diffuse) { m_Diffuse = diffuse; m_Dirty = true;}
	void SetRange	 (float range)			  { m_Range = range > 0.001f ? range : 0.001f; m_Dirty = true;}
	void SetIntensity(float intensity)	      { m_Intensity = intensity; m_Dirty = true;}
	void SetDirty	 (bool b)				  { m_Dirty = b; }

	// ==================================================
	// ----- ライフサイクル -----
	// ==================================================
	void OnAdded() override
	{
		// ここで LightManager へ登録する
		Manager::GetScene()->lightManager().Register(this);

		// Transform キャッシュ
		m_pTransform = Owner()->Transform();
	}
	void Uninit() override
	{
		// ここで LightManager から削除する
		Manager::GetScene()->lightManager().Unregister(this);
	}
};

#endif