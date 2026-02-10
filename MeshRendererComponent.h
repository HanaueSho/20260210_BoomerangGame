/*
	MeshRendererComponent.h
	20250813  hanaue sho
	”どう描くか”という情報を持つ
*/
#ifndef MESHRENDERERCOMPONENT_H_
#define MESHRENDERERCOMPONENT_H_
#include "Component.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MaterialComponent.h"
#include "Renderer.h"

#include "Manager.h"
#include "Scene.h"
#include "LightManager.h"
#include "AnimatorComponent.h"
#include "SkinMatrixProviderComponent.h"


class MeshRendererComponent : public Component
{
protected:
	TransformComponent*	 m_pTransform = nullptr;
	MeshFilterComponent* m_pMeshFilter = nullptr;
	MaterialComponent*	 m_pMaterialBase = nullptr;
	MaterialComponent*	 m_pMaterialOutline = nullptr;

	// SubMesh 用
	std::vector<MaterialComponent*> m_MaterialSlots;
public:
	void SetBaseMaterial(MaterialComponent* mat) { m_pMaterialBase = mat; }
	void SetOutlineMaterial(MaterialComponent* mat) { m_pMaterialOutline = mat; }
	void SetMaterialSlot(int slot, MaterialComponent* mat) { m_MaterialSlots[slot] = mat; }
	MaterialComponent* GetMaterialSlot(int slot) const
	{
		if (slot >= 0 && slot < (int)m_MaterialSlots.size() && m_MaterialSlots[slot])
			return m_MaterialSlots[slot];
		return m_pMaterialBase; // フォールバック
	}
	void ResizeMaterialSlots(int count) { m_MaterialSlots.assign(count, nullptr); }

	// ==================================================
	// ----- ライフサイクル -----
	// ==================================================
	void OnAdded() override
	{
		m_pTransform  = Owner()->GetComponent<TransformComponent>();
		m_pMeshFilter = Owner()->GetComponent<MeshFilterComponent>();
		if (!m_pMaterialBase)
			m_pMaterialBase = Owner()->GetComponent<MaterialComponent>();
		// サブメッシュが未指定ならベースをslot0扱い
		if (m_MaterialSlots.empty() && m_pMaterialBase)
			m_MaterialSlots.push_back(m_pMaterialBase);
	}
	// --------------------------------------------------
	// ベース描画
	// --------------------------------------------------
	virtual void DrawBase() 
	{
		if (!m_pTransform || !m_pMeshFilter || !m_pMaterialBase) return;
		if (!m_pMaterialBase->IsReady() || !m_pMeshFilter->IsReady()) return;

		// マテリアル（シェーダ、IL、テクスチャ、定数バッファのMATERIAL）をバインド
		m_pMaterialBase->Bind();

		// world 行列を Renderer に
		const Matrix4x4& worldMatrix = m_pTransform->WorldMatrix();
		const Matrix4x4& worldMatrixInv = m_pTransform->WorldMatrixInv();
		Renderer::SetObject(worldMatrix, worldMatrixInv);

		// 近傍ポイントライトを更新
		const int k = 4;
		int indexes[k] = {};
		int count = 0;
		count = Manager::GetScene()->lightManager().GetNearestIndexes(m_pTransform->Position(), k, indexes);
		Renderer::SetPointLightIndexes(indexes, count);

		// Animator チェック
		const Matrix4x4* skinPtr = nullptr;
		int boneCount = 0;
		if (auto* provider = Owner()->GetComponent< SkinMatrixProviderComponent>())
		{
			skinPtr = provider->GetSkinMatrixPtr(boneCount);
		}
		else if (auto* animator = Owner()->GetComponent<AnimatorComponent>())
		{
			const auto& skinMats = animator->SkinMatrixes();
			boneCount = (int)skinMats.size();
			skinPtr = (boneCount > 0) ? skinMats.data() : nullptr;
		}
		if (skinPtr && boneCount > 0)
			Renderer::SetSkinMatrixes(skinPtr, boneCount);
		else
		{
			//Renderer::SetSkinMatrixes(nullptr, 0); // 無効化
		}

		// Meshをセットして Draw
		auto* ctx = Renderer::DeviceContext();
		if (!ctx) return;

		UINT stride = m_pMeshFilter->Stride();
		UINT offset = m_pMeshFilter->Offset();
		ID3D11Buffer* vb = m_pMeshFilter->VertexBuffer();

		ctx->IASetPrimitiveTopology(m_pMeshFilter->Topology());
		ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

		if (m_pMeshFilter->IsIndexed())
		{
			ctx->IASetIndexBuffer(m_pMeshFilter->IndexBuffer(), m_pMeshFilter->IndexFormat(), 0);
			if (m_pMeshFilter->HasSubMeshes())
			{
				const int smCount = m_pMeshFilter->SubMeshCount();
				for (int i = 0; i < smCount; i++)
				{
					const SubMesh& sm = m_pMeshFilter->GetSubMesh(i);
					if (sm.indexCount == 0) continue;
					// マテリアルバインド処理
					auto* mat = GetMaterialSlot(sm.materialSlot);
					if (!mat || !mat->IsReady()) mat = m_pMaterialBase;
					mat->Bind();

					ctx->DrawIndexed(sm.indexCount, sm.indexStart, 0);
				}
			}
			else
			{
				ctx->DrawIndexed(m_pMeshFilter->IndexCount(), 0, 0);
			}
		}
		else // 非インデックス描画（頂点描画）
		{
			ctx->Draw(m_pMeshFilter->VertexCount(), 0);
		}
	}
	// --------------------------------------------------
	// アウトライン描画
	// --------------------------------------------------
	void DrawOutline()
	{
		if (!m_pTransform || !m_pMeshFilter || !m_pMaterialOutline) return;
		if (!m_pMaterialOutline->IsReady() || !m_pMeshFilter->IsReady()) return;

		// マテリアル（シェーダ、IL、テクスチャ、定数バッファのMATERIAL）をバインド
		m_pMaterialOutline->Bind();

		// world 行列を Renderer に
		const Matrix4x4& worldMatrix = m_pTransform->WorldMatrix();
		const Matrix4x4& worldMatrixInv = m_pTransform->WorldMatrixInv();
		Renderer::SetObject(worldMatrix, worldMatrixInv);

		// Animator チェック
		const Matrix4x4* skinPtr = nullptr;
		int boneCount = 0;
		if (auto* provider = Owner()->GetComponent< SkinMatrixProviderComponent>())
		{
			skinPtr = provider->GetSkinMatrixPtr(boneCount);
		}
		else if (auto* animator = Owner()->GetComponent<AnimatorComponent>())
		{
			const auto& skinMats = animator->SkinMatrixes();
			boneCount = (int)skinMats.size();
			skinPtr = (boneCount > 0) ? skinMats.data() : nullptr;
		}
		if (skinPtr && boneCount > 0)
			Renderer::SetSkinMatrixes(skinPtr, boneCount);
		else
		{
			//Renderer::SetSkinMatrixes(nullptr, 0); // 無効化
		}

		// Meshをセットして Draw
		auto* ctx = Renderer::DeviceContext();
		if (!ctx) return;

		UINT stride = m_pMeshFilter->Stride();
		UINT offset = m_pMeshFilter->Offset();
		ID3D11Buffer* vb = m_pMeshFilter->VertexBuffer();

		ctx->IASetPrimitiveTopology(m_pMeshFilter->Topology());
		ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

		if (m_pMeshFilter->IsIndexed())
		{
			ctx->IASetIndexBuffer(m_pMeshFilter->IndexBuffer(), m_pMeshFilter->IndexFormat(), 0);

			if (m_pMeshFilter->HasSubMeshes())
			{
				const int smCount = m_pMeshFilter->SubMeshCount();
				for (int i = 0; i < smCount; i++)
				{
					const SubMesh& sm = m_pMeshFilter->GetSubMesh(i);
					if (sm.indexCount == 0) continue;

					ctx->DrawIndexed(sm.indexCount, sm.indexStart, 0);
				}
			}
			else
			{
				ctx->DrawIndexed(m_pMeshFilter->IndexCount(), 0, 0);
			}
		}
		else // 非インデックス描画（頂点描画）
		{
			ctx->Draw(m_pMeshFilter->VertexCount(), 0);
		}
	}
};

#endif