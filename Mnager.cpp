#include "main.h"
#include "manager.h"
#include "scene.h"
#include "game.h"
#include "title.h"
#include "GameMainScene.h"

Scene* Manager::m_pScene = nullptr;
Scene* Manager::m_pSceneNext = nullptr;
float  Manager::s_TimeScale      = 1.0f;
float  Manager::s_HitStopRemain  = 0.0f;
float  Manager::s_HitStopEpsilon = 0.0f;

//_CrtMemState g_S1, g_S2, g_S3;

void Manager::Init()
{
	//_CrtMemCheckpoint(&g_S1);
	//m_pScene = new Game();
	m_pScene = new Title();
	//m_pScene = new GameMainScene();
	m_pScene->Init();
}
void Manager::Uninit()
{
	m_pScene->Uninit();
	delete m_pScene;
	/*_CrtMemCheckpoint(&g_S2);
	_CrtMemState diff;
	if (_CrtMemDifference(&diff, &g_S1, &g_S2)) {
		_CrtMemDumpStatistics(&diff);
	}*/
}
void Manager::Update(float dt)
{
	m_pScene->Update(dt, dt);
}
void Manager::Update(float gameDt, float realDt)
{
	m_pScene->Update(gameDt, realDt);
}
void Manager::FixedUpdate(float dt)
{
	m_pScene->FixedUpdate(dt);
}
void Manager::Draw()
{
	m_pScene->Draw();

	// シーン切り替え
	if (m_pSceneNext != nullptr)
	{
		m_pScene->Uninit();
		delete m_pScene;

		m_pScene = m_pSceneNext;
		m_pScene->Init();

		m_pSceneNext = nullptr;
	}
}

void Manager::SetTimeScale(float s)
{
	if (s < 0.0f) s = 0.0f;
	s_TimeScale = s;
}

void Manager::AddHitStop(float seconds)
{
	if (seconds <= 0.0f) return;
	if (s_HitStopRemain < seconds) // max　推奨
		s_HitStopRemain = seconds;
}

float Manager::CalcGameDt(float realDt)
{
	// 安全クランプ
	if (realDt < 0.0f) realDt = 0.0f;
	if (realDt > 0.05f) realDt = 0.05f; // 50ms 上限（好みで）

	// hitStop 残り時間は realDt で減らす
	if (s_HitStopRemain > 0.0f)
	{
		s_HitStopRemain -= realDt;
		if (s_HitStopRemain < 0.0f) s_HitStopRemain = 0.0f;
	}
	const float effectiveScale = (s_HitStopRemain > 0.0f) ? s_HitStopEpsilon : s_TimeScale;
	return realDt * effectiveScale;
}
