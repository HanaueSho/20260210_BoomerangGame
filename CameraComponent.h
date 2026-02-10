/*
	CameraComponent.h
	20250813  hanaue sho
	カメラに持たせる情報
	今は自作のMatrix4x4ではなくXMMatrixを使っているよ
*/
#ifndef CAMERACOMPONENT_H_
#define CAMERACOMPONENT_H_
#include "Component.h"
#include "TransformComponent.h"
#include "Renderer.h"
#include <DirectXMath.h>
#include "GameObject.h"
#include "Main.h" // SCREEN_SIZE


class CameraComponent : public Component
{
public:
	enum class Mode { Perspective, Ortho2D };

protected:
	// ==================================================
	// ----- 要素 -----
	// ==================================================
	// --------------------------------------------------
	// 依存
	// --------------------------------------------------
	TransformComponent* m_pTransform = nullptr;

	// --------------------------------------------------
	// 共通
	// --------------------------------------------------
	Mode m_Mode		 = Mode::Perspective;
	bool m_IsMain	 = true; // とりあえずメインカメラに設定
	bool m_ProjDirty = true; // 射影パラメータ変更
	Matrix4x4 m_CachedProj{}; // キャッシュ
	Matrix4x4 m_CachedView{}; // キャッシュ

	// --------------------------------------------------
	// 透視投影（Perspective）
	// --------------------------------------------------
	float m_FovY = DirectX::XMConvertToRadians(60.0f);
	float m_Aspect = 16.0f / 9.0f;
	float m_NearZ = 0.1f;
	float m_FarZ = 1000.0f;

	// --------------------------------------------------
	// 平行投影（Orthographic, 左上原点、Y下向き）
	// --------------------------------------------------
	float m_OrthoLeft	= 0.0f;
	float m_OrthoRight  = static_cast<float>(SCREEN_WIDTH);
	float m_OrthoTop	= 0.0f;
	float m_OrthoBottom = static_cast<float>(SCREEN_HEIGHT);
	float m_OrthoNearZ = 0.0f;
	float m_OrthoFarZ = 1.0f;
	bool m_UseTransformIn2D = false; // 2DでもTransformでパン、ズームしたい場合にtrue

public:
	// ==================================================
	// ------ 設定 -----
	// ==================================================
	void SetMode(Mode m) noexcept { if (m_Mode != m) { m_Mode = m; m_ProjDirty = true; } }
	void SetAsMain(bool v) noexcept { m_IsMain = v; }

	// ==================================================
	// ----- 透視投影 -----
	// ==================================================
	void SetPerspective(float fovYRadian, float aspect, float nearZ, float farZ) noexcept
	{
		m_FovY = fovYRadian; m_Aspect = aspect;	m_NearZ = nearZ; m_FarZ = farZ;
		m_ProjDirty = true;
	}
	void SetFovY(float fovY)				 noexcept { m_FovY = fovY; m_ProjDirty = true; }
	void SetAspect(float aspect)			 noexcept { m_Aspect = aspect; m_ProjDirty = true; }
	void SetNearFar(float nearZ, float farZ) noexcept { m_NearZ = nearZ; m_FarZ = farZ; m_ProjDirty = true; }

	// ==================================================
	// ----- 平行投影 -----
	// ==================================================
	void SetOrthoOffCenter(float left, float right, float bottom, float top, float nearZ = 0.0f, float farZ = 1.0f) noexcept
	{
		m_OrthoLeft = left; m_OrthoRight = right; m_OrthoTop = top; m_OrthoBottom = bottom;
		m_OrthoNearZ = nearZ; m_OrthoFarZ = farZ; m_ProjDirty = true;
	}
	void UseTransformIn2D(bool v) noexcept { m_UseTransformIn2D = v; }

	// ==================================================
	// ----- ライフサイクル -----
	// ==================================================
	void OnAdded() override
	{
		m_pTransform = Owner()->GetComponent<TransformComponent>();
		m_ProjDirty = true;
	}
	void Update(float dt) override;
	
	// ==================================================
	// ----- ゲッター -----
	// ==================================================
	bool IsMain() const { return m_IsMain; }
	bool IsModePerspective() const { return m_Mode == Mode::Perspective; }
	bool IsModeortho2D()	 const { return m_Mode == Mode::Ortho2D; }
	const Mode& GetMode()	 const { return m_Mode; }
	const Matrix4x4& View() const { return m_CachedView; }
	const Matrix4x4& Proj() const { return m_CachedProj; }
	float FovY() const { return m_FovY; }

private:
	Matrix4x4 CalcView3D();
	Matrix4x4 CalcProj3D();
	Matrix4x4 CalcView2D();
	Matrix4x4 CalcProj2D();
};

/*
	// 備忘録
	void UpdateView3D()
	{
		if (!m_pTransform) return; // 変更が無いので処理を飛ばす（!m_pTransform->IsWorldDirty()は一旦なしにする）

		// View(LH) :位置とForward/Upから生成
		const Vector3 pos = m_pTransform->Position();
		Vector3 forward = m_pTransform->Forward().normalized();
		Vector3 up = m_pTransform->Up().normalized();
		if (fabsf(Vector3::Dot(forward, up)) > 0.999f) {
			// forward とほぼ平行なら別の軸を選ぶ
			if (fabsf(forward.y) > 0.999f)
			{
				// forward がほぼY軸なら X軸を使う
				up = Vector3(1, 0, 0);
			}
			else
			{
				// それ以外は Y軸を使う
				up = Vector3(0, 1, 0);
			}
		}

		using namespace DirectX;
		XMMATRIX view = XMMatrixLookToLH(
			XMVectorSet(pos.x, pos.y, pos.z, 1.0f),
			XMVectorSet(forward.x, forward.y, forward.z, 0.0f),
			XMVectorSet(up.x, up.y, up.z, 0.0f)
		);

		// Renderer へ
		Renderer::SetViewMatrix(view);

		// Dirty を消費（WorldMatrix() 内部で worldDirty = false にする設計）
		(void)m_pTransform->WorldMatrix();
	}

	void UpdateView2D()
	{
		if (!m_UseTransformIn2D || !m_pTransform)
		{
			// 2D固定カメラ（View = identity）
			Renderer::SetViewMatrix(DirectX::XMMatrixIdentity());
			return;
		}

		// 2Dでパンしたい場合：カメラ位置 C を使って View = Translate(-C)
		const Vector3 pos = m_pTransform->Position();
		const DirectX::XMMATRIX view = DirectX::XMMatrixTranslation(-pos.x, -pos.y, -pos.z);
		Renderer::SetViewMatrix(view);
		(void)m_pTransform->WorldMatrix();
	}

*/


#endif