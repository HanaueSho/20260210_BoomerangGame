/*
	BoneManager.cpp
	20260122  hanaue sho
	モデルのボーンを管理するマネージャ
	モデルオブジェクトが１つ持つ
*/
#include "BoneManager.h"
#include "ModelLoader.h"
#include "Matrix4x4.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "ColliderComponent.h"
#include "RigidbodyComponent.h"
#include "BoneObject.h"
#include "Manager.h"
#include "Scene.h"

namespace
{
	// 行列から Translation を取り出す
	Vector3 ExtractTranslation_RowVector(const Matrix4x4& mat)
	{
		return Vector3(mat.m[3][0], mat.m[3][1], mat.m[3][2]);
	}
}

void BoneManager::UpdateFromAnimator(const std::vector<Matrix4x4>& boneModelPose, const std::vector<char>* disableAnimeWriteMask)
{
	if (!m_pOwner || !m_pSkeleton) return;

	const int boneCount = (int)m_pSkeleton->bones.size();
	if (boneCount <= 0) return;
	if ((int)boneModelPose.size() < boneCount) return;

	const bool useMask = (disableAnimeWriteMask && (int)disableAnimeWriteMask->size() == boneCount); // mask

	// owner のワールド
	TransformComponent* ownerTrans = m_pOwner->Transform();
	const Matrix4x4 ownerWorld = ownerTrans->WorldMatrix();

	// ボーンワールド更新 ＋ Transform 同期
	for (int i = 0; i < boneCount; i++)
	{
		m_BoneWorldPose[i] = boneModelPose[i] * ownerWorld;

		if (useMask && (*disableAnimeWriteMask)[i] != 0) continue; // マスク対象なら上書きスキップ

		// Transform 更新
		if (TransformComponent* bt = m_BoneTransforms[i])
		{
			bt->SetWorldMatrixFromExternal(m_BoneWorldPose[i]);
		}
	}
}

void BoneManager::UpdateFromPhysics(const std::vector<char>* readMask)
{
	if (!m_pSkeleton) return;
	const int boneCount = (int)m_pSkeleton->bones.size();
	if (boneCount <= 0) return;

	const bool useMask = (readMask && (int)readMask->size() == boneCount); // mask

	for (int i = 0; i < boneCount; i++)
	{
		if (useMask && (*readMask)[i] == 0) continue;

		TransformComponent* t = m_BoneTransforms[i];
		if (!t) continue;
		m_BoneWorldPose[i] = t->WorldMatrix();
	}
}

int BoneManager::FindBoneIndex(const std::string& name) const
{
	if (!m_pSkeleton) return -1;
	auto it = m_pSkeleton->boneNameToIndex.find(name);
	if (it == m_pSkeleton->boneNameToIndex.end())
		return -1;
	return it->second;
}

void BoneManager::SetBonePhysics(int boneIndex, bool enable, bool isDynamic)
{
	if (!m_pSkeleton) return;
	const int boneCount = (int)m_pSkeleton->bones.size();
	if (boneIndex < 0 || boneIndex >= boneCount) return;
	
	m_IsPhysicsBone[boneIndex] = enable ? 1 : 0;

	if (!enable) return;

	// ----- Collider と Rigidbody の付与 -----
	GameObject* bone = m_BoneObjects[boneIndex];
	if (!bone->GetComponent<Collider>()) bone->AddComponent<Collider>();
	if (!bone->GetComponent<Rigidbody>())bone->AddComponent<Rigidbody>();
	if (isDynamic) bone->GetComponent<Rigidbody>()->SetBodyTypeDynamic(); 
	else		   bone->GetComponent<Rigidbody>()->SetBodyTypeKinematic(); 
}

bool BoneManager::SetBonePhysics(const std::string& boneName, bool enable, bool isDynamic)
{
	int index = FindBoneIndex(boneName);
	if (index < 0) return false;
	SetBonePhysics(index, enable, isDynamic);
	return true;
}

void BoneManager::SyncBoneScale()
{
	if (!m_pSkeleton || !m_pOwner) return;
	for (int i = 0; i < (int)m_pSkeleton->bones.size(); i++)
	{
		m_BoneTransforms[i]->SetScale(m_pOwner->Transform()->LossyScale());
	}
}

// ボーンのコライダーの自動調整
void BoneManager::AutoSetupCapsulesFromBindPose(int startIndex, int endIndex, float defaultRadius)
{
	if (!m_pSkeleton) return;

	const auto& bones = m_pSkeleton->bones;
	const int boneCount = (int)bones.size();
	if (boneCount <= 0) return;

	// 範囲を丸める
	startIndex = std::max(0, startIndex);
	endIndex = std::min(boneCount - 1, endIndex);
	if (startIndex > endIndex) return;

	// ----- 子リストを作る -----
	std::vector<std::vector<int>> children(boneCount); // 親インデックス → 子インデックス配列
	for (int i = 0; i < boneCount; i++)
	{
		const int p = bones[i].parent;
		if (p >= 0 && p < boneCount) children[p].push_back(i);
	}

	// ----- bindPose のモデル空間行列を作る（bones[] が親→子の順であることが前提） -----
	std::vector<Matrix4x4> bindModel(boneCount);
	for (int i = 0; i < boneCount; i++)
	{
		const int p = bones[i].parent;
		if (p < 0) bindModel[i] = bones[i].bindLocal; // ルートボーン
		else	   bindModel[i] = bones[i].bindLocal * bindModel[p]; // 子×親
	}
	// 位置キャッシュ
	std::vector<Vector3> bindPos(boneCount);
	for (int i = 0; i < boneCount; i++)
		bindPos[i] = ExtractTranslation_RowVector(bindModel[i]);

	// ----- 範囲内ボーンに対して Capsule を自動設定 -----
	const float epsLen = 1e-5f;
	for (int i = startIndex; i <= endIndex; i++)
	{
		GameObject* boneObject = (i >= 0 && i < (int)m_BoneObjects.size()) ? m_BoneObjects[i] : nullptr;
		if (!boneObject) continue;

		// 代表子ボーンを選ぶ（最大距離）
		int bestChild = -1;
		float bestDist = -1.0f;
		const Vector3 p0 = bindPos[i];
		for (int c : children[i])
		{
			const Vector3 p1 = bindPos[c];
			const float d = (p1 - p0).length();
			if (d > bestDist)
			{
				bestDist = d;
				bestChild = c;
			}
		}
		// 子がいない末端骨はスキップ（必要なら球、カプセルへ）
		if (bestChild < 0) {
			//printf("[SKIP] i=%d name=%s\n",i, bones[i].name.c_str());
			continue;
		}

		const Vector3 p1 = bindPos[bestChild];
		const Vector3 seg = (p1 - p0);
		const float len = seg.length();
		if (len < epsLen) continue;

		// dir をボーンローカルへ: childPosLocal = p1 * invBindModel[i]
		Matrix4x4 invBindModel_i = bindModel[i].Inverse();
		Vector3 childPosLocal = invBindModel_i.TransformPoint(p1);
		float childLenLocal = childPosLocal.length();
		if (childLenLocal < epsLen) continue;

		const Vector3 dirLocal = childPosLocal * (1.0f / childLenLocal); // normalize

		// カプセルのローカル＋Yを dirLocal に合わせる
		Quaternion rotLocal = Quaternion::FromToRotation(Vector3(0, 1, 0), dirLocal).normalized();

		// 中点：原点から dirLocal へ len/2 （※ len はモデル空間の長さ）
		// ローカル方向 dirLocal にモデル空間長を掛けるのが気持ち悪ければchildLenLocal を使う手もあるがここでは bindModel のスケールを含んだ長さで統一する
		Vector3 centerLocal = childPosLocal* 0.5f;

		// Collider の取得、追加
		SetBonePhysics(i, true, false);
		Collider* col = boneObject->GetComponent<Collider>();
		if (!col) continue;

		// 半径：既存カプセルがあるならそれを優先、無ければ defaultRadius
		float r = defaultRadius;
		if (auto* cap = dynamic_cast<CapsuleCollision*>(col->Shape()))
		{
			r = cap->Radius();
		}

		// CylinderHeight = len - 2r
		float cylinder = childLenLocal - 2.0f * r;
		if (cylinder < 0.0f) cylinder = 0.0f;

		col->SetCapsule(r, cylinder);

		// オフセット設定（ローカル）
		col->SetOffsetPosition(centerLocal);
		col->SetOffsetRotation(rotLocal);
	}
}

// ボーンの生成
void BoneManager::CreateBoneObjects()
{
	if (!m_pOwner || !m_pSkeleton) return;

	const int boneCount = (int)m_pSkeleton->bones.size();
	if (boneCount <= 0) return;

	m_BoneWorldPose.assign(boneCount, Matrix4x4());
	m_BoneObjects.assign(boneCount, nullptr);
	m_BoneTransforms.assign(boneCount, nullptr);
	m_IsPhysicsBone.assign(boneCount, 0);

	for (int i = 0; i < boneCount; i++)
	{
		const std::string& name = m_pSkeleton->bones[i].name;

		// BoneObject 生成
		BoneObject* bone = Manager::GetScene()->AddGameObject<BoneObject>(1);
		// 名前のセット
		bone->SetBoneIndex(i);
		bone->SetName(name);

		// ----- 登録 -----
		m_BoneObjects[i] = bone;
		m_BoneTransforms[i] = bone->Transform();

		m_IsPhysicsBone[i] = 0;
	}
}

