/*
	BoneManager.h
	20260121  hanaue sho
	モデルのボーンを管理するマネージャ
	モデルオブジェクトが１つ持つ
*/
#ifndef BONEMANAGER_H_
#define BONEMANAGER_H_
#include <vector>
#include <string>

class Skeleton;
class Matrix4x4;
class TransformComponent;
class GameObject;

class BoneManager
{
private:
	// owner
	GameObject* m_pOwner = nullptr;
	// owner のスケルトン
	Skeleton* m_pSkeleton = nullptr;

	// ボーンのワールド空間のマトリクス配列
	std::vector<Matrix4x4> m_BoneWorldPose;
	// ボーン事態の配列
	std::vector<GameObject*> m_BoneObjects;
	// ボーンの Transform 配列（ボーン全体の配列）
	std::vector<TransformComponent*> m_BoneTransforms;
	// ボーンに物理が適用されているか（⇒ Update で更新するか）
	std::vector<char> m_IsPhysicsBone;
public:
	// 初期化
	BoneManager() = delete;
	BoneManager(GameObject* owner, Skeleton* skeleton) : m_pOwner(owner), m_pSkeleton(skeleton) 
	{
		// ボーン生成
		CreateBoneObjects();
		// Scale 同期
		SyncBoneScale();
	}

	// ボーンのワールド空間マトリクスを更新
	void UpdateFromAnimator(const std::vector<Matrix4x4>& boneModelPose, const std::vector<char>* skipMask);
	void UpdateFromPhysics(const std::vector<char>* skipMask);

	// Owner からボーン取得
	int FindBoneIndex(const std::string& name) const;
	const Matrix4x4&    GetBoneWorldMatrix(int boneIndex) const { return m_BoneWorldPose[boneIndex]; }
	TransformComponent* GetBoneTransform(int boneIndex)	  const { return m_BoneTransforms[boneIndex]; }
	GameObject*			GetBoneObject(int boneIndex)	  const { return m_BoneObjects[boneIndex]; }
	int					GetBonesSize()					  const { return (int)m_BoneObjects.size(); }

	// 物理同期のON/OFF
	void SetBonePhysics(int boneIndex, bool enable, bool isDynamic);
	bool SetBonePhysics(const std::string& boneName, bool enable, bool isDynamic);

	// BoneObject の Scale 同期
	void SyncBoneScale();

	// ボーンのコライダーの自動調整
	void AutoSetupCapsulesFromBindPose(int startIndex, int endIndex, float defaultRadius = 1.0f);

private:
	// Skeleton からボーンオブジェクトの生成
	void CreateBoneObjects();
};

#endif