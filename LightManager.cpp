/*
	LightManager.cpp
	20251106  hanaue sho
	ライトの情報を統括してGPUへ送る命令を出す
*/
#include "LightManager.h"
#include "LightComponent.h"
#include "Renderer.h"
#include "Vector3.h"

// --------------------------------------------------
// 初期化、終了処理
// --------------------------------------------------
void LightManager::Init()
{
	m_Components.clear();
	m_Packed.clear();
}
void LightManager::Shutdown()
{
}

// --------------------------------------------------
// フレームの最初にライト情報をパックする
// --------------------------------------------------
void LightManager::BeginFrameAndUploadAllLights()
{
	m_Packed.clear();
	m_Packed.reserve(m_Components.size()); // 連続領域の確保

	for (auto* lc : m_Components)
	{
		if (!lc || !lc->Enable()) continue;

		const auto* tr = lc->Owner()->Transform();
		if (!tr) continue;

		PointLightApp pl{};
		pl.position = tr->Position();
		pl.diffuse  = lc->Diffuse();
		pl.range	= lc->Range();
		pl.intensity = lc->Intensity();

		m_Packed.push_back(pl);
	}

	// バッファ更新
	const int count = static_cast<int>(m_Packed.size());
	Renderer::SetPointLights(count ? m_Packed.data() : nullptr, count);
}

// --------------------------------------------------
// 近傍のライトのインデックスを返す
// --------------------------------------------------
int LightManager::GetNearestIndexes(const Vector3& position, int k, int* outIndex)
{
	if (m_Packed.empty() || k <= 0) return 0;

	struct Slot
	{
		float score = -1e30f;
		int idx = -1;
	};
	std::vector<Slot> top(k);

	for (int i = 0; i < (int)m_Packed.size(); i++)
	{
		const auto& pl = m_Packed[i];
		const Vector3 plPos = pl.position;
		const float range = pl.range;
		const float distSq = (plPos - position).lengthSq();

		const float intensity = pl.intensity;
		const float att = 1.0f / (1.0f + distSq / (range * range));
		const float score = intensity * att; // Score （距離と影響力から算出）

		// 最小スロットと比較
		int minJ = 0;
		for (int j = 1; j < k; j++)
			if (top[j].score < top[minJ].score)
				minJ = j;
		if (score > top[minJ].score) top[minJ] = { score, i };
	}

	// スコアが負のモノは無視
	int count = 0;
	for (int j = 0; j < k; j++)
		if (top[j].idx >= 0)
			outIndex[count++] = top[j].idx;

	return count; // 総数を返す
}

// ストラクチャードバッファ　（魔導書15章）
