/*
	MeshFactory.cpp
	20250818  hanaue sho
	自作メッシュを作る
	作ったらそのまま MeshFilter にセットする（所有権は MeshFilter ）
*/
#define NOMINMAX
#include <algorithm>
#include <cmath>
#include <vector>
#include "MeshFactory.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "MathCommon.h"
#include "Bezier.h"


template<class T> inline T Lerp(T a, T b, T t) { return a + (b - a) * t; }
// --------------------------------------------------
// 平面
// --------------------------------------------------
void MeshFactory::CreateQuad(MeshFilterComponent* filter, const QuadParams& p)
{
	assert(filter);

	VERTEX_3D v[4]{};

	float hw = p.width * (p.originCenter ? 0.5f : 1.0f);
	float hh = p.height * (p.originCenter ? 0.5f : 1.0f);
	float x0 = p.originCenter ? -hw : 0.0f;
	float y0 = p.originCenter ? -hh : 0.0f;

	// ----- 頂点データ作成（左上基準） -----
	v[0].position = { x0		  , y0			 , 0.0f }; v[0].normal = { 0, 0, 1 }; v[0].diffuse = { 1, 1, 1, 1 }; v[0].texcoord = { 0, 0 };
	v[1].position = { x0 + p.width, y0			 , 0.0f }; v[1].normal = { 0, 0, 1 }; v[1].diffuse = { 1, 1, 1, 1 }; v[1].texcoord = { 1, 0 };
	v[2].position = { x0		  , y0 + p.height, 0.0f }; v[2].normal = { 0, 0, 1 }; v[2].diffuse = { 1, 1, 1, 1 }; v[2].texcoord = { 0, 1 };
	v[3].position = { x0 + p.width, y0 + p.height, 0.0f }; v[3].normal = { 0, 0, 1 }; v[3].diffuse = { 1, 1, 1, 1 }; v[3].texcoord = { 1, 1 };

	// ----- VB 作成 -----
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = v;
	ID3D11Buffer* vb = nullptr;
	HRESULT hr = Renderer::Device()->CreateBuffer(&bd, &sd, &vb);
	if (FAILED(hr)) { assert(false && "CreateBuffer VB failed"); return; }

	// ----- IB 作成 -----
	uint32_t idx[6] = { 0, 1, 2, 2, 1, 3 };
	D3D11_BUFFER_DESC ibd{};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(uint32_t) * 6;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA isd{};
	isd.pSysMem = idx;
	ID3D11Buffer* ib = nullptr;
	hr = Renderer::Device()->CreateBuffer(&ibd, &isd, &ib);
	if (FAILED(hr)) { vb->Release(); assert(false && "CreateBuffer IB failed"); return; }


	// ----- MeshFilter に情報を渡す -----
	filter->SetVertexBuffer(vb, sizeof(VERTEX_3D), 4, 0, true);
	filter->SetIndexBuffer(ib, 6, DXGI_FORMAT_R32_UINT, true);
	filter->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void MeshFactory::CreatePlane(MeshFilterComponent* filter, const PlaneParams& p)
{
	assert(filter);
	VERTEX_3D v[4]{};

	float hw = p.width * (p.originCenter ? 0.5f : 1.0f);
	float hd = p.depth * (p.originCenter ? 0.5f : 1.0f);
	float x0 = p.originCenter ? -hw : 0.0f;
	float z0 = p.originCenter ? -hd : 0.0f;

	// ----- 頂点データ作成（左上基準） -----
	v[0].position = { x0		  , 0.0f, z0 + p.depth }; v[2].normal = { 0, 1, 0 }; v[2].diffuse = { 1, 1, 1, 1 }; v[2].texcoord = { 0, 1 };
	v[1].position = { x0 + p.width, 0.0f, z0 + p.depth }; v[3].normal = { 0, 1, 0 }; v[3].diffuse = { 1, 1, 1, 1 }; v[3].texcoord = { 1, 1 };
	v[2].position = { x0		  , 0.0f, z0		   }; v[0].normal = { 0, 1, 0 }; v[0].diffuse = { 1, 1, 1, 1 }; v[0].texcoord = { 0, 0 };
	v[3].position = { x0 + p.width, 0.0f, z0		   }; v[1].normal = { 0, 1, 0 }; v[1].diffuse = { 1, 1, 1, 1 }; v[1].texcoord = { 1, 0 };

	// ----- VB 作成 -----
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = v;
	ID3D11Buffer* vb = nullptr;
	HRESULT hr = Renderer::Device()->CreateBuffer(&bd, &sd, &vb);
	if (FAILED(hr)) { assert(false && "CreateBuffer VB failed"); return; }

	// ----- IB 作成 -----
	uint32_t idx[6] = { 0, 1, 2, 2, 1, 3 };
	D3D11_BUFFER_DESC ibd{};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(uint32_t) * 6;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA isd{};
	isd.pSysMem = idx;
	ID3D11Buffer* ib = nullptr;
	hr = Renderer::Device()->CreateBuffer(&ibd, &isd, &ib);
	if (FAILED(hr)) { vb->Release(); assert(false && "CreateBuffer IB failed"); return; }


	// ----- MeshFilter に情報を渡す -----
	filter->SetVertexBuffer(vb, sizeof(VERTEX_3D), 4, 0, true);
	filter->SetIndexBuffer(ib, 6, DXGI_FORMAT_R32_UINT, true);
	filter->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// --------------------------------------------------
// 球体
// --------------------------------------------------
void MeshFactory::CreateSphere(MeshFilterComponent* filter, const SphereParams& p)
{
	assert(filter);

	// ガード
	const uint32_t slices = std::max<uint32_t>(3, p.slices);
	const uint32_t stacks = std::max<uint32_t>(3, p.stacks);
	const float R = std::max<float>(0.0f, p.radius);
	if (R == 0) return; // 半径０はリターン

	// 頂点数 = (stacks + 1) * (slices + 1)  -----
	// 経度をループで重ね、緯度は端も共有しない方式
	const uint32_t vCount = (stacks + 1) * (slices + 1);
	const uint32_t iCount = stacks * slices * 6; // 各グリッドで２三角形

	std::vector<VERTEX_3D> vertexes;
	std::vector<uint32_t> idx;
	vertexes.resize(vCount);
	idx.reserve(iCount);

	// 緯度：０．．stacks, 経度：０．．slices
	for (uint32_t iy = 0; iy <= stacks; iy++)
	{
		float v = (float)iy / (float)stacks; // [0, 1]
		float theta = (1.0f - v) * PI;		 // [0, π]（下→上）

		float sinT = sinf(theta);
		float cosT = cosf(theta);

		for (uint32_t ix = 0; ix <= slices; ix++)
		{
			float u = (float)ix / (float)slices; // [0, 1]
			float phi = u * 2.0f * PI;			 // [0, ２π]

			float sinP = sinf(phi);
			float cosP = cosf(phi);

			// 単位球の位置
			Vector3 normal = { sinT * cosP, cosT, sinT * sinP }; // Y が上の球
			Vector3 pos = normal * R;

			if (p.insideOut) normal = -normal; // 内向き

			// VERTEX_3D へ
			VERTEX_3D vert{};
			vert.position = pos;
			vert.normal = normal;
			vert.diffuse = Vector4(1, 1, 1, 1);
			vert.texcoord = Vector2(u, theta / PI); // vが逆なので

			vertexes[iy * (slices + 1) + ix] = vert;
		}
	}

	// インデックス -----
	// 通常：(i0, i2, i1), (i2, i3, i1)
	auto emitTri = [&](uint32_t a, uint32_t b, uint32_t c)
		{
			if (!p.insideOut) { idx.push_back(a); idx.push_back(b); idx.push_back(c); }
			else { idx.push_back(a); idx.push_back(c); idx.push_back(b); }
		};
	auto emitQuad = [&](uint32_t i00, uint32_t i01, uint32_t i10, uint32_t i11)
		{
			emitTri(i00, i10, i01);
			emitTri(i10, i11, i01);
		};

	for (uint32_t iy = 0; iy < stacks; iy++)
	{
		for (uint32_t ix = 0; ix < slices; ix++)
		{
			uint32_t i00 = iy		* (slices + 1) + ix;
			uint32_t i01 = iy		* (slices + 1) + (ix + 1);
			uint32_t i10 = (iy + 1) * (slices + 1) + ix;
			uint32_t i11 = (iy + 1) * (slices + 1) + (ix + 1);

			emitQuad(i00, i01, i10, i11);
		}
	}

	// ----- VB 作成 -----
	D3D11_BUFFER_DESC vbd{};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = (UINT)(sizeof(VERTEX_3D) * vertexes.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vsd{};
	vsd.pSysMem = vertexes.data();
	ID3D11Buffer* vb = nullptr;
	HRESULT hr = Renderer::Device()->CreateBuffer(&vbd, &vsd, &vb);
	if (FAILED(hr)) { assert(false && "CreateBuffer VB failed"); return; }

	// ----- IB 作成 -----
	D3D11_BUFFER_DESC ibd{};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = (UINT)(sizeof(uint32_t) * idx.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA isd{};
	isd.pSysMem = idx.data();
	ID3D11Buffer* ib = nullptr;
	hr = Renderer::Device()->CreateBuffer(&ibd, &isd, &ib);
	if (FAILED(hr)) { assert(false && "CreateBuffer IB failed"); return; }

	// ----- MeshFilter へ -----
	filter->SetVertexBuffer(vb, sizeof(VERTEX_3D), (UINT)vertexes.size(), 0, true);
	filter->SetIndexBuffer (ib, (UINT)idx.size(), DXGI_FORMAT_R32_UINT, true);
	filter->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// --------------------------------------------------
// 立方体
// --------------------------------------------------
void MeshFactory::CreateCube(MeshFilterComponent* filter, const CubeParams& p)
{
	assert(filter);

	const float w = std::max(0.0f, p.size.x);
	const float h = std::max(0.0f, p.size.y);
	const float d = std::max(0.0f, p.size.z);

	// 原点オフセット
	const float x0 = p.originCenter ? -w * 0.5f : 0.0f;
	const float y0 = p.originCenter ? -h * 0.5f : 0.0f;
	const float z0 = p.originCenter ? -d * 0.5f : 0.0f;

	const float x1 = x0 + w;
	const float y1 = y0 + h;
	const float z1 = z0 + d;

	// 面ごと４頂点
	VERTEX_3D v[24]{};

	auto setV = [&](int i, float px, float py, float pz, const Vector3& n, float u, float vv)
		{
			v[i].position = Vector3(px, py, pz);
			Vector3 nn = p.insideOut ? -n : n;
			v[i].normal   = Vector3(nn.x, nn.y, nn.z);
			v[i].diffuse  = Vector4(1, 1, 1, 1);
			v[i].texcoord = Vector2(u, vv);
		};

	// +X
	setV( 0, x1, y1, z0, { +1, 0, 0 }, 0, 0 );
	setV( 1, x1, y1, z1, { +1, 0, 0 }, 1, 0 );
	setV( 2, x1, y0, z0, { +1, 0, 0 }, 0, 1 );
	setV( 3, x1, y0, z1, { +1, 0, 0 }, 1, 1 );

	// -X
	setV( 4, x0, y1, z1, { -1, 0, 0 }, 0, 0 );
	setV( 5, x0, y1, z0, { -1, 0, 0 }, 1, 0 );
	setV( 6, x0, y0, z1, { -1, 0, 0 }, 0, 1 );
	setV( 7, x0, y0, z0, { -1, 0, 0 }, 1, 1 );

	// +Y
	setV( 8, x0, y1, z1, { 0, +1, 0 }, 0, 0 );
	setV( 9, x1, y1, z1, { 0, +1, 0 }, 1, 0 );
	setV(10, x0, y1, z0, { 0, +1, 0 }, 0, 1 );
	setV(11, x1, y1, z0, { 0, +1, 0 }, 1, 1 );

	// -Y
	setV(12, x1, y0, z1, { 0, -1, 0 }, 0, 0);
	setV(13, x0, y0, z1, { 0, -1, 0 }, 1, 0);
	setV(14, x1, y0, z0, { 0, -1, 0 }, 0, 1);
	setV(15, x0, y0, z0, { 0, -1, 0 }, 1, 1);

	// +Z
	setV(16, x1, y1, z1, { 0, 0, +1 }, 0, 0);
	setV(17, x0, y1, z1, { 0, 0, +1 }, 1, 0);
	setV(18, x1, y0, z1, { 0, 0, +1 }, 0, 1);
	setV(19, x0, y0, z1, { 0, 0, +1 }, 1, 1);

	// -Z
	setV(20, x0, y1, z0, { 0, 0, -1 }, 0, 0);
	setV(21, x1, y1, z0, { 0, 0, -1 }, 1, 0);
	setV(22, x0, y0, z0, { 0, 0, -1 }, 0, 1);
	setV(23, x1, y0, z0, { 0, 0, -1 }, 1, 1);

	// インデックス
	uint32_t idx[36]{}; // （各面２三角形、合計３６）
	auto putFace = [&](int baseV, int baseI)
		{
			if (!p.insideOut) // (0,1,2), (2,1,3) 
			{
				idx[baseI + 0] = baseV + 0; idx[baseI + 1] = baseV + 1; idx[baseI + 2] = baseV + 2;
				idx[baseI + 3] = baseV + 2; idx[baseI + 4] = baseV + 1; idx[baseI + 5] = baseV + 3;
			}
			else // 反転
			{
				idx[baseI + 0] = baseV + 0; idx[baseI + 1] = baseV + 2; idx[baseI + 2] = baseV + 1;
				idx[baseI + 3] = baseV + 2; idx[baseI + 4] = baseV + 3; idx[baseI + 5] = baseV + 1;
			}
		};

	putFace( 0,  0); // +X
	putFace( 4,  6); // -X
	putFace( 8, 12); // +Y
	putFace(12, 18); // -Y
	putFace(16, 24); // +Z
	putFace(20, 30); // -Z

	// ----- VB 作成 -----
	D3D11_BUFFER_DESC vbd{};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(VERTEX_3D) * 24;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vsd{};
	vsd.pSysMem = v;

	ID3D11Buffer* vb = nullptr;
	HRESULT hr = Renderer::Device()->CreateBuffer(&vbd, &vsd, &vb);
	if (FAILED(hr)) { assert(false && "CreateBuffer VB failed"); return; }

	// ----- IB 作成 -----
	D3D11_BUFFER_DESC ibd{};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(uint32_t) * 36;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA isd{};
	isd.pSysMem = idx;

	ID3D11Buffer* ib = nullptr;
	hr = Renderer::Device()->CreateBuffer(&ibd, &isd, &ib);
	if (FAILED(hr)) { vb->Release(); assert(false && "CreateBuffer IB failed"); return; }

	// ----- MeshFilter へ -----
	filter->SetVertexBuffer(vb, sizeof(VERTEX_3D), 24, 0, true);
	filter->SetIndexBuffer(ib, 36, DXGI_FORMAT_R32_UINT, true); 
	filter->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// --------------------------------------------------
// 円柱
// --------------------------------------------------
void MeshFactory::CreateCylinder(MeshFilterComponent* filter, const CylinderParams& p)
{
	assert(filter);

	const uint32_t slices = std::max(3, p.slices);
	const uint32_t stacks = std::max(0, p.stacks);
	const float R = std::max(0.0f, p.radius);
	const float H = std::max(0.0f, p.height);
	const float yBottom = -H * 0.5f;
	const float yTop	= +H * 0.5f;

	// ----- 頂点バッファ構築 -----
	// 胴：(stacks + 1) * (slices + 1)
	const uint32_t vBody   = (stacks + 1) * (slices + 1);
	const uint32_t vTop	   = 1 + (slices + 1);
	const uint32_t vBottom = 1 + (slices + 1);
	const uint32_t vCount  = vBody + vTop + vBottom;

	std::vector<VERTEX_3D> vertexes(vCount);

	auto putVertex = [&](uint32_t i, const Vector3& pos, const Vector3& n, float u, float vv)
		{
			vertexes[i].position = pos;
			vertexes[i].normal   = p.insideOut ? -n : n;
			vertexes[i].diffuse  = Vector4(1, 1, 1, 1);
			vertexes[i].texcoord = Vector2(u, vv);
		};

	uint32_t idxV = 0;

	// 胴体頂点 -----
	for (uint32_t iy = 0; iy <= stacks; iy++)
	{
		float t = (stacks == 0) ? 0.0f : (float)iy / (float)stacks; // [0, 1]
		float yu = Lerp(yBottom, yTop, t);
		float v = 1.0f - t; // テクスチャｖ（上に行くほど小さく）

		for (uint32_t ix = 0; ix <= slices; ix++)
		{
			float u = (float)ix / (float)slices; // [0, 1]
			float phi = u * 2.0f * PI;
			float cx = cosf(phi), sx = sinf(phi);
			Vector3 pos = { R * cx, yu, R * sx };
			Vector3 nor = { cx, 0, sx };		 // 胴は水平法線
			putVertex(idxV++, pos, nor, u, v);  // 頂点情報の登録 
		}
	}

	// 上キャップ -----
	const uint32_t baseTop = idxV;
	putVertex(idxV++, { 0, yTop, 0 }, { 0, +1, 0 }, 0.5f, 0.5f); // 上面のセンター部
	for (uint32_t ix = 0; ix <= slices; ix++)
	{
		float u = (float)ix / (float)slices;
		float phi = u * 2.0f * PI;
		float cx = cosf(phi), sx = sinf(phi);
		Vector3 pos = { R * cx, yTop, R * sx };
		// 極座標っぽいUV
		float uu = 0.5f + 0.5f * cx;
		float vv = 0.5f - 0.5f * sx;
		putVertex(idxV++, pos, { 0,+1,0 }, uu, vv); // 頂点情報登録
	}

	// 下キャップ -----
	const uint32_t baseBottom = idxV;
	putVertex(idxV++, { 0, yBottom, 0 }, { 0, -1, 0 }, 0.5f, 0.5f); // 上面のセンター部
	for (uint32_t ix = 0; ix <= slices; ix++)
	{
		float u = (float)ix / (float)slices;
		float phi = u * 2.0f * PI;
		float cx = cosf(phi), sx = sinf(phi);
		Vector3 pos = { R * cx, yBottom, R * sx };
		// 極座標っぽいUV
		float uu = 0.5f + 0.5f * cx;
		float vv = 0.5f - 0.5f * sx;
		putVertex(idxV++, pos, { 0,-1,0 }, uu, vv); // 頂点情報登録
	}

	// ----- インデックス -----
	std::vector<uint32_t> idx;
	idx.reserve(stacks * slices * 6 + slices * 3 * 2);

	auto emitTri = [&](uint32_t a, uint32_t b, uint32_t c)
		{
			if (!p.insideOut) { idx.push_back(a); idx.push_back(b); idx.push_back(c); } // 逆にしたら直った（なんで？）
			else { idx.push_back(a); idx.push_back(c); idx.push_back(b); }
		};
	auto emitQuad = [&](uint32_t i00, uint32_t i01, uint32_t i10, uint32_t i11)
		{
			emitTri(i00, i10, i01);
			emitTri(i10, i11, i01);
		};
	
	// 胴体：リング間の四角
	for (uint32_t iy = 0; iy < stacks; iy++)
	{
		for (uint32_t ix = 0; ix < slices; ix++)
		{
			uint32_t i00 =  iy		* (slices + 1) +  ix;
			uint32_t i01 =  iy		* (slices + 1) + (ix + 1);
			uint32_t i10 = (iy + 1) * (slices + 1) +  ix;
			uint32_t i11 = (iy + 1) * (slices + 1) + (ix + 1);
			emitQuad(i00, i01, i10, i11);
		}
	}

	// 上キャップ：扇
	for (uint32_t ix = 0; ix < slices; ix++)
	{
		uint32_t c = baseTop;
		uint32_t r0 = baseTop + 1 + ix;
		uint32_t r1 = baseTop + 1 + ix + 1;
		emitTri(c, r1, r0); // (c, r1, r0)で表向き
	}
	// 下キャップ：扇
	for (uint32_t ix = 0; ix < slices; ix++)
	{
		uint32_t c = baseBottom;
		uint32_t r0 = baseBottom + 1 + ix;
		uint32_t r1 = baseBottom + 1 + ix + 1;
		emitTri(c, r0, r1); // 下向きなので(c, r0, r1)で表向き
	}

	// チェック -----
	// 頂点法線と面法線が同じ向きを向いているか判定
	auto checkFace = [&](uint32_t a, uint32_t b, uint32_t c) {
		Vector3 pa = vertexes[a].position;
		Vector3 pb = vertexes[b].position;
		Vector3 pc = vertexes[c].position;

		Vector3 e1 = pb - pa;
		Vector3 e2 = pc - pa;
		Vector3 fN = Vector3::Cross(e1, e2).normalized(); // 面法線
		Vector3 vN = vertexes[a].normal.normalized();	  // 頂点法線
		float s = Vector3::Dot(fN, vN);
		return (p.insideOut ? (s < 0.f) : (s > 0.f));
		};
	assert(checkFace(idx[0], idx[1], idx[2]) && "Winding/normal mismatch");

	// ----- VB 作成 -----
	D3D11_BUFFER_DESC vbd{};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = (UINT)(sizeof(VERTEX_3D) * vertexes.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vsd{};
	vsd.pSysMem = vertexes.data();
	ID3D11Buffer* vb = nullptr;
	HRESULT hr = Renderer::Device()->CreateBuffer(&vbd, &vsd, &vb);
	if (FAILED(hr)) { assert(false && "VB failed"); return; }

	// ----- IB 作成 -----
	D3D11_BUFFER_DESC ibd{};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = (UINT)(sizeof(uint32_t) * idx.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA isd{};
	isd.pSysMem = idx.data();
	ID3D11Buffer* ib = nullptr;
	hr = Renderer::Device()->CreateBuffer(&ibd, &isd, &ib);
	if (FAILED(hr)) { vb->Release(); assert(false && "IB failed"); return; }

	// ----- MeshFilter へ -----
	filter->SetVertexBuffer(vb, sizeof(VERTEX_3D), (UINT)vertexes.size(), 0, true);
	filter->SetIndexBuffer(ib, (UINT)idx.size(), DXGI_FORMAT_R32_UINT, true);
	filter->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// --------------------------------------------------
// カプセル
// --------------------------------------------------
void MeshFactory::CreateCapsule(MeshFilterComponent* filter, const CapsuleParams& p)
{
	assert(filter);

	const uint32_t slices  = std::max(3, p.slices);
	const uint32_t stacksB = std::max(0, p.stacksBody);
	const uint32_t stacksC = std::max(1, p.stacksCap);
	const float R = std::max(0.0f, p.radius);
	const float H = std::max(0.0f, p.cylinderHeight);

	const float yBody0 = -H * 0.5f; // 胴体下端
	const float yBody1 = +H * 0.5f; // 胴体上端

	// ----- リング定義（ｙと半径）を順に積む -----
	struct Ring { float y; float r; bool cap; }; // cap == true なら 半球面の法線（球中心基準）
	std::vector<Ring> rings;

	// 下半球
	for (uint32_t i = 0; i <= stacksC; i++)
	{
		float t = (float)i / (float)stacksC; // 0..1
		float theta = t * (PI * 0.5f);		 // 0..pi/2
		float cy = -cosf(theta);			 // [-1..0]
		float sy =  sinf(theta);			 // [0..1]
		rings.push_back({ yBody0 + cy * R, sy * R, true }); // 球中心は yBody0
	}
	// 胴体：内部リングのみ（境界は半球が持っている）
	for (uint32_t i = 1; i < stacksB; i++)
	{
		float t = (float)i / (float)stacksB; // 0..1
		float y = Lerp(yBody0, yBody1, t);
		rings.push_back({ y, R, false });
	}
	// 上半球
	for (int i = (int)stacksC; i >= 0; i--)
	{
		float t = (float)i / (float)stacksC; // 0..1
		float theta = t * (PI * 0.5f);		 // 0..pi/2
		float cy = cosf(theta);			 // [0..1]
		float sy = sinf(theta);			 // [0..1]
		rings.push_back({ yBody1 + cy * R, sy * R, true }); // 球中心は yBody1
	}

	//  ----- 頂点配列（各リングに slices + 1個）-----
	const uint32_t vPerRing = slices + 1;
	const uint32_t vCount = (uint32_t)rings.size() * vPerRing;
	std::vector<VERTEX_3D> vertexes(vCount);
	
	std::vector<float> vRing(rings.size(), 0.0f); // 各リングのｖを先に決める
	const float Lcap = 0.5f * PI * R; // 半球弧
	const float Ltotal = std::max(1e-8f, H + 2.0f * Lcap); // 上から下までの長さ
	const float epsY = 1e-6f;
	auto clamp01 = [](float x) { return std::max(0.0f, std::min(1.0f, x)); };
	for (uint32_t ir = 0; ir < rings.size(); ir++)
	{
		const Ring& rg = rings[ir];
		float s = 0.0f;
		if (rg.cap) // 半球部分
		{
			float theta = asinf(clamp01((R > 0.0f) ? (rg.r / R) : 0.0f)); // 0..pi/2
			if (rg.y <= yBody0 + epsY) // 下半球
				s = R * theta;
			else // 上半球
				s = H + 2.0f * Lcap - R * theta; // H + πR - Rθ
		}
		else // 胴体部分 
			s = Lcap + (rg.y - yBody0);

		vRing[ir] = 1.0f - s / Ltotal;
	}


	auto putVertex = [&](uint32_t i, const Vector3& pos, const Vector3& n, float u, float vv)
		{
			vertexes[i].position = pos;
			vertexes[i].normal   = p.insideOut ? -n : n;
			vertexes[i].diffuse  = Vector4(1, 1, 1, 1);
			vertexes[i].texcoord = Vector2(u, vv);
		};

	for (uint32_t ir = 0; ir < rings.size(); ir++)
	{
		const Ring& r = rings[ir];
		for (uint32_t ix = 0; ix <= slices; ix++)
		{
			float u = (float)ix / (float)slices;
			float phi = u * 2.0f * PI;
			float cx = cosf(phi), sx = sinf(phi);
			Vector3 pos = { r.r * cx, r.y, r.r * sx };

			Vector3 n;
			if (r.cap) // 半径：球中心基準の法線
			{
				float centerY = (r.y > 0.0f) ? yBody1 : yBody0;
				n = Vector3(pos.x, pos.y - centerY, pos.z).normalized();
			}
			else // 胴体：水平法線
			{
				n = Vector3(cx, 0, sx);
			}
			putVertex(ir* vPerRing + ix, pos, n, u, vRing[ir]);
		}
	}

	// ----- インデックス -----
	std::vector<uint32_t> idx;
	idx.reserve((uint32_t)( (rings.size() - 1) * slices * 6 ));

	auto emitTri = [&](uint32_t a, uint32_t b, uint32_t c)
		{
			if (!p.insideOut) { idx.push_back(a); idx.push_back(b); idx.push_back(c); }
			else { idx.push_back(a); idx.push_back(c); idx.push_back(b); }
		};
	auto emitQuad = [&](uint32_t i00, uint32_t i01, uint32_t i10, uint32_t i11)
		{
			emitTri(i00, i10, i01);
			emitTri(i10, i11, i01);
		};

	for (uint32_t r = 0; r + 1 < rings.size(); r++)
	{
		for (uint32_t ix = 0; ix < slices; ix++)
		{
			uint32_t i00 = r * vPerRing + ix;
			uint32_t i01 = r * vPerRing + (ix + 1);
			uint32_t i10 = (r + 1) * vPerRing + ix;
			uint32_t i11 = (r + 1) * vPerRing + (ix + 1);
			emitQuad(i00, i01, i10, i11);
		}
	}

	// チェック -----
	auto checkFace = [&](uint32_t a, uint32_t b, uint32_t c) {
		Vector3 pa = vertexes[a].position;
		Vector3 pb = vertexes[b].position;
		Vector3 pc = vertexes[c].position;

		Vector3 e1 = pb - pa;
		Vector3 e2 = pc - pa;
		Vector3 n = Vector3::Cross(e1, e2);
		float len = n.length();

		// 極の縮退（３点がほぼ共通／同一点）はスキップ
		const float kAreaEps = 1e-8f;
		if (len < kAreaEps) return true; // 面積ほぼゼロ（三角形潰れ）

		n /= len; // 正規化

		Vector3 na = vertexes[a].normal;
		Vector3 nb = vertexes[b].normal;
		Vector3 nc = vertexes[c].normal;

		const float kDotEps = -1e-4f; // 数値誤差を許容
		float sa = Vector3::Dot(n, na);
		float sb = Vector3::Dot(n, nb);
		float sc = Vector3::Dot(n, nc);

		// insideOut でも巻き順も法線も同じ側を向く設計なので「常に正」を期待
		return (sa > kDotEps && sb > kDotEps && sc > kDotEps);
		};
	assert(checkFace(idx[0], idx[1], idx[2]) && "Winding/normal mismatch");


	// ----- VB 作成 -----
	D3D11_BUFFER_DESC vbd{};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = (UINT)(sizeof(VERTEX_3D) * vertexes.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vsd{};
	vsd.pSysMem = vertexes.data();
	ID3D11Buffer* vb = nullptr;
	HRESULT hr = Renderer::Device()->CreateBuffer(&vbd, &vsd, &vb);
	if (FAILED(hr)) { assert(false && "VB failed"); return; }

	// ----- IB 作成 -----
	D3D11_BUFFER_DESC ibd{};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = (UINT)(sizeof(uint32_t) * idx.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA isd{};
	isd.pSysMem = idx.data();
	ID3D11Buffer* ib = nullptr;
	hr = Renderer::Device()->CreateBuffer(&ibd, &isd, &ib);
	if (FAILED(hr)) { vb->Release(); assert(false && "IB failed"); return; }

	// ----- MeshFilter へ -----
	filter->SetVertexBuffer(vb, sizeof(VERTEX_3D), (UINT)vertexes.size(), 0, true);
	filter->SetIndexBuffer(ib, (UINT)idx.size(), DXGI_FORMAT_R32_UINT, true);
	filter->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// --------------------------------------------------
// メッシュフィールド
// --------------------------------------------------
void MeshFactory::CreateMeshField(MeshFilterComponent* filter, const MeshFieldParams& p)
{
	assert(filter);

	const int width = p.width; // グリッド頂点数
	const int depth = p.depth; // グリッド頂点数
	const float csX = p.cellSizeX;
	const float csZ = p.cellSizeZ;

	assert(width >= 2);
	assert(depth >= 2);
	assert(static_cast<int>(p.heights.size()) == width * depth);

	const int vertexCount = width * depth;
	const int quadCount = (width - 1) * (depth - 1);
	const int indexCount = quadCount * 6;

	// ----- 頂点バッファ作成 -----
	std::vector<VERTEX_3D> vertexes(vertexCount);

	for (int iz = 0; iz < depth; iz++)
	{
		for (int ix = 0; ix < width; ix++)
		{
			int idx = iz * width + ix; // HeightMap と同じ引数の取り方をする

			float px = ix * csX;
			float pz = iz * csZ;
			float py = p.heights[idx];
			
			VERTEX_3D& vertex = vertexes[idx];
			vertex.position = { px, py, pz };
			
			// 白色
			vertex.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

			// UV 
			if (!p.textureSpread)
			{
				float u = (width > 1) ? (float)ix / (float)(width - 1) : 0.0f;
				float v = (depth > 1) ? (float)iz / (float)(depth - 1) : 0.0f;
				vertex.texcoord = { u, v };
			}
			else // セルごとに１枚テクスチャを貼る
			{
				// wrap サンプラ
				vertex.texcoord = { (float)ix, (float)iz };
			}
			// normal は後で計算するのでここではゼロ初期化
			vertex.normal = { 0.0f, 0.0f, 0.0f };
		}
	}

	// 面法線を集計して頂点法線を作る
	auto AccumulateFaceNormal = [&](int i0, int i1, int i2)
		{
			const Vector3& p0 = vertexes[i0].position;
			const Vector3& p1 = vertexes[i1].position;
			const Vector3& p2 = vertexes[i2].position;

			// CW が表になるように Cross の向きを合わせる
			Vector3 e1 = p1 - p0;
			Vector3 e2 = p2 - p0;
			Vector3 n = Vector3::Cross(e1, e2);

			vertexes[i0].normal += n;
			vertexes[i1].normal += n;
			vertexes[i2].normal += n;
		};
	for (int iz = 0; iz < depth - 1; iz++)
	{
		for (int ix = 0; ix < width - 1; ix++)
		{
			int v00 = iz * width + ix;		 // 左手前
			int v10 = v00 + 1;				 // 右手前
			int v01 = (iz + 1) * width + ix; // 左奥
			int v11 = v01 + 1;				 // 右奥

			AccumulateFaceNormal(v00, v01, v11);
			AccumulateFaceNormal(v00, v11, v10);
		}
	}
	// 各頂点の法線を正規化
	for (int i = 0; i < vertexCount; i++)
	{
		vertexes[i].normal.normalize();
	}

	// ----- インデックスバッファ作成 -----
	std::vector<uint32_t> indexes;
	indexes.resize(indexCount);

	int idx = 0;
	for (int iz = 0; iz < depth - 1; iz++)
	{
		for (int ix = 0; ix < width - 1; ix++)
		{
			int v00 = iz * width + ix;		 // 左手前
			int v10 = v00 + 1;				 // 右手前
			int v01 = (iz + 1) * width + ix; // 左奥
			int v11 = v01 + 1;				 // 右奥

			// tri0: v00, v01, v11
			indexes[idx++] = v00;
			indexes[idx++] = v01;
			indexes[idx++] = v11;
			// tri1: v00, v11, v10
			indexes[idx++] = v00;
			indexes[idx++] = v11;
			indexes[idx++] = v10;
		}
	}

	// ----- VB 作成 -----
	D3D11_BUFFER_DESC vbd{};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = (UINT)(sizeof(VERTEX_3D) * vertexes.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vsd{};
	vsd.pSysMem = vertexes.data();
	ID3D11Buffer* vb = nullptr;
	HRESULT hr = Renderer::Device()->CreateBuffer(&vbd, &vsd, &vb);
	if (FAILED(hr)) { assert(false && "VB failed"); return; }

	// ----- IB 作成 -----
	D3D11_BUFFER_DESC ibd{};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = (UINT)(sizeof(uint32_t) * indexes.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA isd{};
	isd.pSysMem = indexes.data();
	ID3D11Buffer* ib = nullptr;
	hr = Renderer::Device()->CreateBuffer(&ibd, &isd, &ib);
	if (FAILED(hr)) { vb->Release(); assert(false && "IB failed"); return; }

	// ----- MeshFilter へ -----
	filter->SetVertexBuffer(vb, sizeof(VERTEX_3D), (UINT)vertexes.size(), 0, true);
	filter->SetIndexBuffer(ib, (UINT)indexes.size(), DXGI_FORMAT_R32_UINT, true);
	filter->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// --------------------------------------------------
// リンゴ
// --------------------------------------------------
void MeshFactory::CreateApple(MeshFilterComponent* filter, const AppleParams& p)
{
	assert(filter);

	// ガード
	const uint32_t slices = std::max<uint32_t>(3, p.slices);
	const uint32_t stacks = std::max<uint32_t>(3, p.stacks);
	const float R = std::max<float>(0.0f, p.radius);
	if (R == 0) return; // 半径０はリターン

	// 断面ベジェ生成 -----
	Bezier bezier;
	bezier.CreatePoint(Vector3(0.00f * R, -1.00f * R, 0)); // 
	bezier.CreatePoint(Vector3(1.30f * R, -1.10f * R, 0)); // 
	bezier.CreatePoint(Vector3(0.90f * R, 0.00f * R, 0)); // 
	bezier.CreatePoint(Vector3(1.50f * R, 0.90f * R, 0)); // 
	bezier.CreatePoint(Vector3(0.50f * R, 1.00f * R, 0)); // 
	bezier.CreatePoint(Vector3(0.00f * R, 0.70f * R, 0)); // 
	bezier.CreatePoint(Vector3(0.00f * R, 0.50f * R, 0)); // 

	// 導関数近似 （ベジェ曲線の接ベクトルを求める）-----
	auto BezierDeriv = [&](float t)->Vector3
		{
			// 端点の数値安定を少し良くするクランプ＋中心差分
			t = std::min(1.0f, std::max(0.0f, t));
			const float dt = 1e-3f;
			float t0 = std::max(0.0f, t - dt);
			float t1 = std::min(1.0f, t + dt);
			Vector3 p0 = bezier.GetValue(t0);
			Vector3 p1 = bezier.GetValue(t1);
			float inv = (t1 - t0) > 0 ? 1.0f / (t1 - t0) : 0.0f;
			return (p1 - p0) * inv; // XY に有効（Zは常に０）
		};

	// ｔサンプル列＆弧長パラメータｖ（縦ストレッチ抑制）-----
	std::vector<float> tSamples(stacks + 1);
	for (uint32_t i = 0; i <= stacks; i++) tSamples[i] = (float)i / (float)stacks;

	// 粗積分で弧長（累積長）→Vに正規化
	const int integN = 512; // ０～１の区間を分割する解像度
	std::vector<float> cum(integN + 1, 0.0f);
	{
		Vector3 prev = bezier.GetValue(0.0f);
		for (int i = 1; i <= integN; i++)
		{
			float tt = (float)i / integN;
			Vector3 curr = bezier.GetValue(tt);
			Vector3 d = curr - prev;
			cum[i] = cum[i - 1] + d.length();
			prev = curr;
		}
		if (cum.back() <= 1e-8f)
			for (int i = 0; i <= integN; i++) cum[i] = (float)i / integN;
	}

	// 任意の点 t を弧長に基づく０～１の割り合いに写像
	auto ArcV = [&](float t)->float
		{
			float ft = t * integN;
			int i0 = (int)std::floor(ft);
			int i1 = std::min(i0 + 1, integN);
			float a = ft - i0;
			float c0 = cum[i0], c1 = cum[i1];
			float total = cum.back();
			float c = (1 - a) * c0 + a * c1;
			return total > 0.0f ? (c / total) : t;
		};
	std::vector<float> vParam(stacks + 1);
	for (uint32_t i = 0; i <= stacks; i++) vParam[i] = ArcV(tSamples[i]);

	// 周方向角度テーブル -----
	std::vector<float> sinP(slices + 1), cosP(slices + 1);
	for (uint32_t ix = 0; ix <= slices; ix++)
	{
		float u = (float)ix / (float)slices;
		float phi = u * 2.0f * PI;
		sinP[ix] = std::sinf(phi);
		cosP[ix] = std::cosf(phi);
	}

	// 頂点生成 -----
	const uint32_t vCount = (stacks + 1) * (slices + 1);
	const uint32_t iCount = stacks * slices * 6;
	std::vector<VERTEX_3D> vertexes(vCount);

	// 正規化関数
	auto Normalize2 = [](float x, float y)->Vector2 {
		float l = std::sqrt(x * x + y * y);
		if (l <= 1e-8f) return Vector2(1, 0);
		return Vector2(x / l, y / l);
		};

	for (uint32_t iy = 0; iy <= stacks; iy++)
	{
		float t = tSamples[iy];

		Vector3 P = bezier.GetValue(t); // (x, y, 0)
		Vector3 dP = BezierDeriv(t);	 // (dx, dy, 0)

		float r = std::max(0.0f, P.x);
		float y = P.y;

		// 法線→外向き法線
		Vector2 T2 = Normalize2(dP.x, dP.y);
		Vector2 N2 = Normalize2(+T2.y, -T2.x);

		for (uint32_t ix = 0; ix <= slices; ix++)
		{
			float cx = cosP[ix], sx = sinP[ix];

			Vector3 pos(r * cx, y, r * sx);		// 位置
			Vector3 n(N2.x * cx, N2.y, N2.x * sx); // 法線

			float nl = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
			if (nl < 1e-6f) { n = Vector3(cx, 0, sx); nl = 1.0f; }
			n.x /= nl; n.y /= nl; n.z /= nl;
			if (p.insideOut) { n.x = -n.x; n.y = -n.y; n.z = -n.z; }

			float U = (float)ix / (float)slices; // 周方向
			float Vv = 1.0f - vParam[iy];				 // 縦（弧長基準）

			VERTEX_3D vert{};
			vert.position = pos;
			vert.normal   = n;
			vert.diffuse  = Vector4(1, 1, 1, 1);
			vert.texcoord = Vector2(U, Vv);

			vertexes[iy * (slices + 1) + ix] = vert;
		}
	}

	// インデックス -----
	std::vector<uint32_t> idx; idx.reserve(iCount);
	auto emitTri = [&](uint32_t a, uint32_t b, uint32_t c)
		{
			if (!p.insideOut) { idx.push_back(a); idx.push_back(b); idx.push_back(c); }
			else { idx.push_back(a); idx.push_back(c); idx.push_back(b); }
		};
	auto emitQuad = [&](uint32_t i00, uint32_t i01, uint32_t i10, uint32_t i11)
		{
			emitTri(i00, i10, i01);
			emitTri(i10, i11, i01);
		};
	for (uint32_t iy = 0; iy < stacks; iy++)
	{
		for (uint32_t ix = 0; ix < slices; ix++)
		{
			uint32_t i00 = iy * (slices + 1) + ix;
			uint32_t i01 = iy * (slices + 1) + (ix + 1);
			uint32_t i10 = (iy + 1) * (slices + 1) + ix;
			uint32_t i11 = (iy + 1) * (slices + 1) + (ix + 1);
			emitQuad(i00, i01, i10, i11);
		}
	}

	// ----- VB 作成 -----
	D3D11_BUFFER_DESC vbd{};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = (UINT)(sizeof(VERTEX_3D) * vertexes.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vsd{};
	vsd.pSysMem = vertexes.data();
	ID3D11Buffer* vb = nullptr;
	HRESULT hr = Renderer::Device()->CreateBuffer(&vbd, &vsd, &vb);
	if (FAILED(hr)) { assert(false && "VB failed"); return; }

	// ----- IB 作成 -----
	D3D11_BUFFER_DESC ibd{};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = (UINT)(sizeof(uint32_t) * idx.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA isd{};
	isd.pSysMem = idx.data();
	ID3D11Buffer* ib = nullptr;
	hr = Renderer::Device()->CreateBuffer(&ibd, &isd, &ib);
	if (FAILED(hr)) { vb->Release(); assert(false && "IB failed"); return; }

	// ----- MeshFilter へ -----
	filter->SetVertexBuffer(vb, sizeof(VERTEX_3D), (UINT)vertexes.size(), 0, true);
	filter->SetIndexBuffer(ib, (UINT)idx.size(), DXGI_FORMAT_R32_UINT, true);
	filter->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
static_assert(sizeof(VERTEX_3D) == 48, "VERTEX_3D layout mismatch (pos3 + nrm3 + col4 + uv2)");