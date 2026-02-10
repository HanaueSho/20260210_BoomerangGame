/*
	DebugRendereComponent.h
	20251130  hanaue sho
	コライダーのワイヤーフレーム表示など
*/
#include <cmath>
#include <cstdio>
#include <io.h>
#include <cstring>
#include <memory>
#include <cassert>
#include "DebugRenderer.h"
#include "Renderer.h"

void DebugRenderer::Initialize()
{
	// コンテナの領域確保
	m_Vertexes.reserve(MaxVertexes);
	m_Indexes.reserve(MaxIndexes);

	// 頂点バッファ作成
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth	  = sizeof(DebugVertex) * MaxVertexes;
	bd.Usage		  = D3D11_USAGE_DYNAMIC;
	bd.BindFlags	  = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	Renderer::Device()->CreateBuffer(&bd, nullptr, m_VertexBuffer.GetAddressOf());

	// インデックスバッファ作成
	D3D11_BUFFER_DESC ibd = {};
	ibd.ByteWidth	   = sizeof(int) * MaxIndexes;
	ibd.Usage		   = D3D11_USAGE_DYNAMIC;
	ibd.BindFlags	   = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	Renderer::Device()->CreateBuffer(&ibd, nullptr, m_IndexBuffer.GetAddressOf());

	// シェーダー作成
	// ファイルネームから探す
	FILE* file = fopen("shader\\VS_Debug.cso", "rb");
	assert(file);
	long int fsize = _filelength(_fileno(file));
	std::unique_ptr<unsigned char[]> buffer(new unsigned char[fsize]);
	fread(buffer.get(), fsize, 1, file);
	fclose(file);

	Renderer::Device()->CreateVertexShader(buffer.get(), fsize, NULL, m_VS.GetAddressOf());

	// DebugDraw 用の InputLayout を作る
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	 0,	(UINT)offsetof(DebugVertex, pos)  , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetof(DebugVertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = _countof(layout);

	Renderer::Device()->CreateInputLayout(layout,
										  numElements,
										  buffer.get(),
										  fsize,
										  m_IL.GetAddressOf()
										 );
	// PS
	FILE* filePS = fopen("shader\\PS_Debug.cso", "rb");
	assert(filePS);
	long int fsizePS = _filelength(_fileno(filePS));
	std::unique_ptr<unsigned char[]> bufferPS(new unsigned char[fsizePS]);
	fread(bufferPS.get(), fsizePS, 1, filePS);
	fclose(filePS);

	Renderer::Device()->CreatePixelShader(bufferPS.get(), fsizePS, NULL, m_PS.GetAddressOf());
}

void DebugRenderer::Begin()
{
	m_Vertexes.clear();
	m_Indexes.clear();
}

void DebugRenderer::Flush()
{
	if (m_Vertexes.empty() || m_Indexes.empty()) return;

	auto* ctx = Renderer::DeviceContext();

	// VB に頂点データを書き込む
	D3D11_MAPPED_SUBRESOURCE mapped;
	ctx->Map(m_VertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, m_Vertexes.data(), sizeof(DebugVertex) * m_Vertexes.size());
	ctx->Unmap(m_VertexBuffer.Get(), 0);

	// IB にインデックスを書き込む
	ctx->Map(m_IndexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, m_Indexes.data(), sizeof(int) * m_Indexes.size());
	ctx->Unmap(m_IndexBuffer.Get(), 0);

	// パイプラインにセット
	UINT stride = sizeof(DebugVertex);
	UINT offset = 0;
	ID3D11Buffer* vb = m_VertexBuffer.Get();

	ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	ctx->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	ctx->IASetInputLayout(m_IL.Get());

	ctx->VSSetShader(m_VS.Get(), nullptr, 0);
	ctx->PSSetShader(m_PS.Get(), nullptr, 0);

	// Draw
	ctx->DrawIndexed((UINT)m_Indexes.size(), 0, 0);
}


void DebugRenderer::DrawLine(const Vector3& p0, const Vector3& p1, const Vector4& color)
{
	// 今の頂点数を控える
	int baseIndex = (int)m_Vertexes.size();

	// 頂点を２つ追加
	m_Vertexes.push_back({ p0, color });
	m_Vertexes.push_back({ p1, color });

	// インデックスを２つ追加
	m_Indexes.push_back(baseIndex + 0);
	m_Indexes.push_back(baseIndex + 1);
}

void DebugRenderer::DrawBox(const Vector3& center, const Vector3& halfSize, const Matrix4x4& rotation, const Vector4& color)
{
	// 現在の頂点数を控える
	int baseIndex = (int)m_Vertexes.size();

	// ８頂点を追加
	Vector3 local[8] =
	{
		{-halfSize.x, -halfSize.y, -halfSize.z},
		{ halfSize.x, -halfSize.y, -halfSize.z},
		{ halfSize.x,  halfSize.y, -halfSize.z},
		{-halfSize.x,  halfSize.y, -halfSize.z},
		{-halfSize.x, -halfSize.y,  halfSize.z},
		{ halfSize.x, -halfSize.y,  halfSize.z},
		{ halfSize.x,  halfSize.y,  halfSize.z},
		{-halfSize.x,  halfSize.y,  halfSize.z},
	};
	for (int i = 0; i < 8; i++)
	{
		Vector3 p = local[i];
		p = rotation.TransformNormal(p); // 回転を適用
		p += center; // 中心を足してワールド座標へ

		m_Vertexes.push_back({ p, color });
	}

	// １２本のエッジをインデックスとして追加
	auto AddEdge = [&](int a, int b)
		{
			m_Indexes.push_back(baseIndex + a);
			m_Indexes.push_back(baseIndex + b);
		};

	AddEdge(0, 1); AddEdge(1, 2); AddEdge(2, 3); AddEdge(3, 0);
	AddEdge(4, 5); AddEdge(5, 6); AddEdge(6, 7); AddEdge(7, 4);
	AddEdge(0, 4); AddEdge(1, 5); AddEdge(2, 6); AddEdge(3, 7);
}

void DebugRenderer::DrawSphere(const Vector3& center, float radius, const Vector4& color)
{
	Matrix4x4 rot = {};
	DrawSphere(center, radius, rot, color);
}
void DebugRenderer::DrawSphere(const Vector3& center, float radius, const Matrix4x4& rotation, const Vector4& color)
{
	const int segments = 32;

	auto LocalToWorld = [&](const Vector3& local)
		{
			Vector3 p = rotation.TransformNormal(local);
			p += center;
			return p;
		};

	auto DrawCircle = [&](int axis)
		{
			int baseIndex = (int)m_Vertexes.size();

			for (int i = 0; i < segments; i++)
			{
				float  t = (float)i / segments * 2.0f * PI;
				float ct = std::cosf(t);
				float st = std::sinf(t);

				Vector3 local = {0.0f, 0.0f, 0.0f};

				switch (axis)
				{
				case 0: // XY
					local.x += radius * ct;
					local.y += radius * st;
					break;
				case 1: // YZ
					local.y += radius * ct;
					local.z += radius * st;
					break;
				case 2: // ZX
					local.z += radius * ct;
					local.x += radius * st;
					break;
				}
				Vector3 p = LocalToWorld(local);

				m_Vertexes.push_back({ p, color });

				if (i > 0)
				{
					m_Indexes.push_back(baseIndex + i - 1);
					m_Indexes.push_back(baseIndex + i);
				}
			}
			// 最後と最初を結ぶ
			m_Indexes.push_back(baseIndex + segments - 1);
			m_Indexes.push_back(baseIndex + 0);
		};

	DrawCircle(0);
	DrawCircle(1);
	DrawCircle(2);
}

void DebugRenderer::DrawCapsule(const Vector3& center, float radius, float cylinderHeight, const Matrix4x4& rotation, const Vector4& color)
{
	const float halfHeight = cylinderHeight * 0.5f;

	auto LocalToWorld = [&](const Vector3& local)
		{
			Vector3 p = rotation.TransformNormal(local);
			p += center;
			return p;
		};
	// ローカル
	Vector3 bottomLocal = { 0.0f, -halfHeight, 0.0f };
	Vector3 topLocal = { 0.0f, halfHeight, 0.0f };

	// ワールド
	Vector3 bottomWorld = LocalToWorld(bottomLocal);
	Vector3 topWorld	= LocalToWorld(topLocal);

	// ボトム、トップ部分は球体を描画
	DrawSphere(bottomWorld, radius, rotation, color);
	DrawSphere(topWorld,    radius, rotation, color);

	// シリンダー部分を描画
	Vector3 dirs[4] =
	{
		{  radius, 0.0f,  0.0f},
		{ -radius, 0.0f,  0.0f},
		{  0.0f,   0.0f,  radius},
		{  0.0f,   0.0f, -radius},
	};

	for (int i = 0; i < 4; i++)
	{
		Vector3 bottom = bottomLocal + dirs[i];
		Vector3 top    = topLocal	 + dirs[i];

		Vector3 p0 = LocalToWorld(bottom);
		Vector3 p1 = LocalToWorld(top);

		DrawLine(p0, p1, color);
	}
}
