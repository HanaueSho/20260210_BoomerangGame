/*
	MeshFilterComponent.h
	20250813  hanaue sho
	ジオメトリ（形状）の保持者
	”何を描くか”という情報を持つ（頂点、インデックス、トポロジ etc.）
*/
#ifndef MESHFILTERCOMPONENT_H_
#define MESHFILTERCOMPONENT_H_
#include "Component.h"
#include <d3d11.h>
#include <vector>
#include <cstdint>

// モデルをパーツごとにDrawするためのもの
struct SubMesh
{
	uint32_t indexStart = 0;   // IB の開始オフセット
	uint32_t indexCount = 0;   // そのサブメッシュのインデックス数
	int		 materialSlot = 0; // 使用するマテリアル番号
};

class MeshFilterComponent : public Component
{
protected:
	ID3D11Buffer* m_VertexBuffer = nullptr;
	UINT		  m_Stride		= 0;
	UINT		  m_Offset		= 0;
	UINT		  m_VertexCount = 0;

	ID3D11Buffer* m_IndexBuffer = nullptr;
	DXGI_FORMAT	  m_IndexFormat = DXGI_FORMAT_R32_UINT;
	UINT		  m_IndexCount  = 0;

	D3D11_PRIMITIVE_TOPOLOGY m_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	std::vector<SubMesh> m_SubMeshes;
	int m_MaterialSlotCount = 0;

	// 所有権
	bool m_OwnsVB = false;
	bool m_OwnsIB = false;

public:
	void SetVertexBuffer(ID3D11Buffer* vb, UINT stride, UINT vertexCount, UINT offset = 0, bool takeOwnership = false) noexcept
	{ m_VertexBuffer = vb; m_Stride = stride; m_VertexCount = vertexCount; m_Offset = offset; m_OwnsVB = takeOwnership; }

	void SetIndexBuffer(ID3D11Buffer* ib, UINT indexCount, DXGI_FORMAT format = DXGI_FORMAT_R32_UINT, bool takeOwnership = false) noexcept
	{ m_IndexBuffer = ib; m_IndexCount = indexCount; m_IndexFormat = format; m_OwnsIB = takeOwnership; }
	
	void SetTopology(D3D11_PRIMITIVE_TOPOLOGY topo) noexcept
	{ m_Topology = topo; }

	void ClearSubMeshes() { m_SubMeshes.clear(); }
	void AddSubMesh(const SubMesh& sm) { m_SubMeshes.push_back(sm); }
	void SetMaterialSlotCount(int count) { m_MaterialSlotCount = count; }
	int  MaterialSlotCount() const { return m_MaterialSlotCount; }

	// アクセス
	ID3D11Buffer* VertexBuffer() const noexcept  { return m_VertexBuffer; }
	ID3D11Buffer* IndexBuffer()  const noexcept  { return m_IndexBuffer; }
	UINT		  Stride()		 const noexcept  { return m_Stride; }
	UINT		  Offset()		 const noexcept  { return m_Offset; }
	UINT		  VertexCount()	 const noexcept  { return m_VertexCount; }
	UINT		  IndexCount()	 const noexcept  { return m_IndexCount; }
	DXGI_FORMAT	  IndexFormat()	 const noexcept  { return m_IndexFormat; }
	D3D11_PRIMITIVE_TOPOLOGY Topology() const noexcept { return m_Topology; }
	int			   SubMeshCount()	 const { return (int)m_SubMeshes.size(); }
	const SubMesh& GetSubMesh(int i) const { return m_SubMeshes[i]; }
	bool	       HasSubMeshes()	 const { return !m_SubMeshes.empty(); }

	// 妥当性
	bool IsIndexed() const noexcept { return m_IndexBuffer != nullptr && m_IndexCount > 0; }

	bool IsReady() const noexcept
	{
		if (!m_VertexBuffer || m_Stride == 0) return false;
		if (IsIndexed())
		{
			if (m_IndexFormat != DXGI_FORMAT_R16_UINT && m_IndexFormat != DXGI_FORMAT_R32_UINT) return false;
		}
		else
		{
			if (m_VertexCount == 0) return false;
		}
		return true;
	}

	// 破棄（所有時のみ）
	void Uninit() override
	{
		if (m_OwnsVB && m_VertexBuffer) { m_VertexBuffer->Release(); m_VertexBuffer = nullptr; }
		if (m_OwnsIB && m_IndexBuffer) { m_IndexBuffer->Release(); m_IndexBuffer = nullptr; }
		m_SubMeshes.clear();
	}

	// 外部が寿命を管理する場合のリセット
	void ResetNonOwning() noexcept
	{
		m_VertexBuffer = nullptr; m_IndexBuffer = nullptr;
		m_Stride = m_Offset = m_VertexCount = m_IndexCount = 0;
		m_IndexFormat = DXGI_FORMAT_R32_UINT;
		m_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		m_OwnsVB = m_OwnsIB = false;
		m_SubMeshes.clear();
	}
};

#endif