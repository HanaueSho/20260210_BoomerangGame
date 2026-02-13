#include <algorithm> // std::stable_sort
#include "scene.h"
#include "manager.h"
#include "renderer.h"

#include "camera.h"
#include "TransformComponent.h"
#include "MeshRendererComponent.h"
#include "MeshFilterComponent.h"
#include "MaterialComponent.h"
#include "SpriteRendererComponent.h"
#include "CameraComponent.h"

#include "PhysicsSystem.h"
#include "LightManager.h"
#include "Texture.h"

#include "Keyboard.h"
#include "DebugRenderer.h"

Scene::~Scene() = default; // cpp で定義しないとリンカーエラーが出る

void Scene::Init()
{
	m_pPhysicsSystem.reset(new PhysicsSystem(*this)); // make_unique は deleter には使えない
	m_pPhysicsSystem->Init();
	m_pLightManager.reset(new LightManager(*this)); // make_unique は deleter には使えない
	m_pLightManager->Init();
	Texture::PreloadDirectory("assets\\texture", true);
}

void Scene::Uninit()
{
	for (auto& gameObjectList : m_GameObjects)
	{
		for (auto* gameObject : gameObjectList)
		{
			if (!gameObject) continue;

			gameObject->Uninit();
			delete gameObject;
		}
		gameObjectList.clear();
	}

	if (m_pPhysicsSystem) { m_pPhysicsSystem->Shutdown(); m_pPhysicsSystem.reset(); }
	if (m_pLightManager)  { m_pLightManager->Shutdown();  m_pLightManager.reset(); }
}

void Scene::Update(float dt)
{
	m_pLightManager->BeginFrameAndUploadAllLights(); // ライト情報を格納する
	// オブジェクト更新処理
	for (auto& gameObjectList : m_GameObjects)
	{
		for (auto* gameObject : gameObjectList)
		{
			if (gameObject)
				gameObject->Update(dt);
		}
	}
	// オブジェクト破棄処理
	for (auto& gameObjectList : m_GameObjects) // 参照型じゃないとバグるよ
	{
		gameObjectList.remove_if(
			[](GameObject* object)
			{
				if (object && object->IsDestroyRequested())
				{
					object->Uninit();
					delete object; // 所有者が delete
					return true; // リストから外す
				}
				return false;
			});
	}

	// デバッグ切り替え
	if (Keyboard_IsKeyDownTrigger(KK_D0))
	{
		m_IsDrawCollision = !m_IsDrawCollision;
	}
	if (Keyboard_IsKeyDownTrigger(KK_D9))
	{
		m_IsDrawCollisionInFront = !m_IsDrawCollisionInFront;
	}
}

void Scene::FixedUpdate(float fixedDt)
{
	for (auto& gameObjectList : m_GameObjects)
	{
		for (auto* gameObject : gameObjectList)
		{
			if (gameObject)
				gameObject->FixedUpdate(fixedDt);
		}
	}

	// ----- 物理演算を１ステップ -----
	m_pPhysicsSystem->BeginStep(fixedDt);
	m_pPhysicsSystem->Step(fixedDt);
	m_pPhysicsSystem->EndStep(fixedDt);
}

void Scene::Draw()
{
	// ----- メインカメラ取得（３D／UI） -----
	CameraComponent* camera3D = nullptr;
	CameraComponent* cameraUI = nullptr;
	auto cameras = GetGameObjects<Camera>();
	for (auto* c : cameras)
	{
		if (!c) continue;
		CameraComponent* cc = c->GetComponent<CameraComponent>();
		if (!cc || !cc->IsMain()) continue; // カメラコンポーネントを持っていない or メインカメラじゃない

		if (cc->IsModePerspective())
			camera3D = cc;
		else if (cc->IsModeortho2D())
			cameraUI = cc;
	}

	// 深度ソート用
	Vector3 cameraPos{}, cameraFwd{ 0, 0, 1 };
	if (camera3D)
	{
		cameraPos = camera3D->Owner()->GetComponent<TransformComponent>()->Position();
		cameraFwd = camera3D->Owner()->GetComponent<TransformComponent>()->Forward().normalized();
	}

	// ----- 描画するオブジェクトの構造体 -----
	struct DrawItem
	{
		MeshRendererComponent* mr = nullptr;
		MaterialComponent* mat = nullptr;
		float				   depth = 0.0f;
	};
	struct UIDrawItem
	{
		SpriteRendererComponent* sr = nullptr;
		int order = 0;
		bool onWorld = false;
	};
	std::vector<DrawItem> opaques;      // 不透明
	std::vector<DrawItem> transparents; // 透明
	std::vector<UIDrawItem> uiSprites; // UI用
	opaques.reserve(512);      // 領域確保
	transparents.reserve(512); // 領域確保
	uiSprites.reserve(256);

	// ----- 描画対象を収集 -----
	for (auto& gameObjectList : m_GameObjects)
	{
		for (auto* gameObject : gameObjectList)
		{
			if (!gameObject) continue;

			// ----- UIDrawItem -----
			if (auto* sr = gameObject->GetComponent<SpriteRendererComponent>())
			{
				if (sr->IsUI())
				{
					uiSprites.push_back({ sr, sr->Order(), sr->OnWorld() });
					continue;
				}
			}

			// ----- DrawItem -----
			auto* mr = gameObject->GetComponent<MeshRendererComponent>();
			auto* mat = gameObject->GetComponent<MaterialComponent>();
			auto* tf = gameObject->GetComponent<TransformComponent>();
			if (!mr || !mat || !tf) continue;

			// 視線方向で深度を出す
			float d = 0.0f;
			if (camera3D)
			{
				const Vector3 objectPos = tf->Position();
				d = Vector3::Dot(objectPos - cameraPos, cameraFwd);
			}

			DrawItem item{ mr, mat, d };
			if (mat->IsTransparent()) transparents.push_back(item);
			else					  opaques.push_back(item);
		}
	}

	// ----- ソート -----
	// 不透明： front-to-back（小さいdepth＝手前から）
	std::stable_sort(opaques.begin(), opaques.end(),
		[](const DrawItem& a, const DrawItem& b) {return a.depth < b.depth; });
	// 透明：　back-to-front（大きいdepth＝奥から）
	std::stable_sort(transparents.begin(), transparents.end(),
		[](const DrawItem& a, const DrawItem& b) {return a.depth > b.depth; });
	// UI：　SR の Order 順
	std::stable_sort(uiSprites.begin(), uiSprites.end(),
		[](const UIDrawItem& a, const UIDrawItem& b) {return a.order < b.order; });

	// ----- 描画 -----
	// カメラセット（３D空間）
	if (camera3D)
	{
		auto* tf = camera3D->Owner()->GetComponent<TransformComponent>();
		Vector3 pos = tf ? tf->Position() : Vector3();
		Renderer::SetCamera(camera3D->View(), camera3D->Proj(), pos, camera3D->FovY(), SCREEN_WIDTH, SCREEN_HEIGHT);
	}
	// 不透明（depth書き込みON, ブレンドOFF）
	Renderer::SetRasterizerState(Renderer::RasterizerMode::BackCullSolid);
	Renderer::SetBlendState(Renderer::BlendMode::Opaque);
	Renderer::SetDepthState(Renderer::DepthMode::Opaque);
	for (auto& it : opaques) it.mr->DrawBase();
	// 不透明アウトライン
	Renderer::SetRasterizerState(Renderer::RasterizerMode::FrontCullSolid);
	Renderer::SetBlendState(Renderer::BlendMode::Opaque);
	Renderer::SetDepthState(Renderer::DepthMode::OutlineLE);
	for (auto& it : opaques) { it.mr->DrawOutline(); }
	// 透明（depth書き込みOFF推奨, ブレンドON）
	Renderer::SetRasterizerState(Renderer::RasterizerMode::BackCullSolid);
	Renderer::SetBlendState(Renderer::BlendMode::Premultiplied);
	Renderer::SetDepthState(Renderer::DepthMode::Transparent);
	for (auto& it : transparents) it.mr->DrawBase();
	// ワールド空間UI（デプス無視）
	Renderer::SetRasterizerState(Renderer::RasterizerMode::BackCullSolid);
	Renderer::SetBlendState(Renderer::BlendMode::Premultiplied);
	Renderer::SetDepthState(Renderer::DepthMode::DisableDepth);
	for (auto& it : uiSprites) { if(it.onWorld) it.sr->DrawBase(); }

	// デバッグ描画
	if (m_IsDrawCollision)
	{
		auto* dr = Renderer::GetDebugRenderer();
		dr->Begin();
		Renderer::SetRasterizerState(Renderer::RasterizerMode::WireFrame);
		Renderer::SetBlendState(Renderer::BlendMode::Opaque);
		if (m_IsDrawCollisionInFront)Renderer::SetDepthState(Renderer::DepthMode::DisableDepth);
		else						 Renderer::SetDepthState(Renderer::DepthMode::OutlineLE);
		m_pPhysicsSystem->DrawDebug(*dr);
		dr->Flush();
	}
	// カメラセット（UI）
	if (cameraUI)
	{
		auto* tf = cameraUI->Owner()->GetComponent<TransformComponent>();
		Vector3 pos = tf ? tf->Position() : Vector3();
		Renderer::SetCamera(cameraUI->View(), cameraUI->Proj(), pos, cameraUI->FovY(), SCREEN_WIDTH, SCREEN_HEIGHT);

		// UI 描画
		Renderer::SetRasterizerState(Renderer::RasterizerMode::BackCullSolid);
		Renderer::SetBlendState(Renderer::BlendMode::Premultiplied);
		Renderer::SetDepthState(Renderer::DepthMode::DisableDepth);
		for (auto& it : uiSprites) { if (!it.onWorld) it.sr->DrawBase(); }
	}
}

void PhysicsSystemDeleter::operator()(PhysicsSystem* p) noexcept
{
	delete p;
}

void LightManagerDeleter::operator()(LightManager* p) noexcept
{
	delete p;
}