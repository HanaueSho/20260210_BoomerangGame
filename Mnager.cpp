#include "main.h"
#include "manager.h"
#include "scene.h"
#include "game.h"
#include "title.h"

Scene* Manager::m_pScene = nullptr;
Scene* Manager::m_pSceneNext = nullptr;

void Manager::Init()
{
	//m_pScene = new Game();
	m_pScene = new Title();
	m_pScene->Init();

}


void Manager::Uninit()
{
	m_pScene->Uninit();
	delete m_pScene;
}

void Manager::Update(float dt)
{
	m_pScene->Update(dt);
}

void Manager::FixedUpdate(float dt)
{
	m_pScene->FixedUpdate(dt);
}

void Manager::Draw()
{
	m_pScene->Draw();

	// ƒV[ƒ“Ø‚è‘Ö‚¦
	if (m_pSceneNext != nullptr)
	{
		m_pScene->Uninit();
		delete m_pScene;

		m_pScene = m_pSceneNext;
		m_pScene->Init();

		m_pSceneNext = nullptr;
	}
}
