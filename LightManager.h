/*
	LightManager.h
	20251105  hanaue sho
	ライトの情報を統括してGPUへ送る命令を出す
*/
#ifndef LIGHTMANAGER_H_
#define LIGHTMANAGER_H_
#include <vector>

struct PointLightApp;

class LightComponent;
class Vector3;
class Scene;

class LightManager
{
private:
	// ==================================================
	// ----- 構成要素 -----
	// ==================================================
	// --------------------------------------------------
	// 親シーン
	// --------------------------------------------------
	Scene* m_pScene = nullptr; // 現在のシーン
	// --------------------------------------------------
	// 登録された LightComponent
	// --------------------------------------------------
	std::vector<LightComponent*> m_Components;
	// --------------------------------------------------
	// 毎フレーム再構築する連続配列
	// --------------------------------------------------
	std::vector<PointLightApp> m_Packed;

public:
	LightManager(Scene& scene) { m_pScene = &scene; }
	~LightManager() = default;

	// --------------------------------------------------
	// 初期化、終了処理
	// --------------------------------------------------
	void Init();
	void Shutdown();

	// --------------------------------------------------
	// 登録
	// --------------------------------------------------
	void Register(LightComponent* lc) { m_Components.push_back(lc); }
	// --------------------------------------------------
	// 登録解除
	// --------------------------------------------------
	void Unregister(LightComponent* lc) 
	{
		auto it = std::find(m_Components.begin(), m_Components.end(), lc);
		if (it != m_Components.end()) m_Components.erase(it);
	}

	// --------------------------------------------------
	// フレームの最初にライト情報をパックする
	// --------------------------------------------------
	void BeginFrameAndUploadAllLights();
	// --------------------------------------------------
	// 近傍のライトのインデックスを返す
	// --------------------------------------------------
	int GetNearestIndexes(const Vector3& position, int k, int* outIndex);
};

#endif