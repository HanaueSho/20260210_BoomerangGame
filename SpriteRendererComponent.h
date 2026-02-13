/*
	SpriteRendererComponent.h
	20260209  hanaue sho
*/
#ifndef SPRITERENDERERCOMPONENT_H_
#define SPRITERENDERERCOMPONENT_H_
#include "Component.h"
#include "MeshRendererComponent.h"
#include "Vector2.h"
#include "Vector4.h"

class SpriteRendererComponent : public MeshRendererComponent
{
private:
	Vector2 m_UvOffset = { 0, 0 };
	Vector2 m_UvScale  = { 1, 1 };
	Vector4 m_Color	   = { 1, 1, 1, 1 };

	bool m_IsUI    = false; // UI表示
	bool m_OnWorld = false; // ３D空間にあるかどうか
	int m_Order = 0; // UI描画順序（深度には頼らない）

public:
	// ライフサイクル
	void OnAdded() override
	{
		MeshRendererComponent::OnAdded();
	}
	void DrawBase() override
	{
		if (!m_pTransform || !m_pMeshFilter || !m_pMaterialBase) return;
		if (!m_pMaterialBase->IsReady() || !m_pMeshFilter->IsReady()) return;

		// マテリアル（シェーダ、IL、テクスチャ、定数バッファのMATERIAL）をバインド
		m_pMaterialBase->Bind();

		// world 行列を Renderer に
		const Matrix4x4& worldMatrix = m_pTransform->WorldMatrix();
		const Matrix4x4& worldMatrixInv = m_pTransform->WorldMatrixInv();
		Renderer::SetObject(worldMatrix, worldMatrixInv);
		
		// Sprite 情報更新
		SpriteApp s;
		s.spriteUVOffset = m_UvOffset;
		s.spriteUVScale  = m_UvScale;
		s.spriteColor	 = m_Color;
		Renderer::SetSprite(s);
		
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
			ctx->DrawIndexed(m_pMeshFilter->IndexCount(), 0, 0);
		}
		else // 非インデックス描画（頂点描画）
		{
			ctx->Draw(m_pMeshFilter->VertexCount(), 0);
		}
	}

	// セッター
	void SetUVRect(const Vector2& offset, const Vector2& scale)
	{
		m_UvOffset = offset;
		m_UvScale = scale;
	}
	void SetColor(const Vector4& color) { m_Color = color; }
	void SetUI(bool b) { m_IsUI = b; }
	void SetOnWorld(bool b) { m_OnWorld = b; }
	void SetOrder(int order) { m_Order = order; }
	bool IsUI() const { return m_IsUI; }
	bool OnWorld() const { return m_OnWorld; }
	int Order() const { return m_Order; }
};

#endif