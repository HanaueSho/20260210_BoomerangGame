#pragma once

class Scene;

class Manager
{
private:
	static Scene* m_pScene;
	static Scene* m_pSceneNext;

public:
	static void Init();
	static void Uninit();
	static void Update(float dt);
	static void FixedUpdate(float dt);
	static void Draw();

	static Scene* GetScene() { return m_pScene; }

	template <typename T>
	static void SetScene()
	{
		m_pSceneNext = new T;
	}

};