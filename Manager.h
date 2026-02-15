#pragma once

class Scene;

class Manager
{
private:
	static Scene* m_pScene;
	static Scene* m_pSceneNext;

	// 時間状態
	static float s_TimeScale;
	static float s_HitStopRemain;
	static float s_HitStopEpsilon; // ０で完全停止、0.001fで超スロー停止

public:
	static void Init();
	static void Uninit();
	static void Update(float dt);
	static void Update(float gameDt, float realDt);
	static void FixedUpdate(float dt);
	static void Draw();

	static Scene* GetScene() { return m_pScene; }

	template <typename T>
	static void SetScene()
	{
		m_pSceneNext = new T;
	}

	// 時間制御
	static void SetTimeScale(float s); // s < 1.0f でスロー
	static void AddHitStop(float second); // second の時間ヒットストップ
	static float CalcGameDt(float realDt);// Mainループから呼ぶ：realDtを渡すとgameDtを返す
};