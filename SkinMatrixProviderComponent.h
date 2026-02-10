/*
	SkinMatrixProviderComponent.h
	20260123  hanaue sho
*/
#ifndef SKINMATRIXPROVIDERCOMPONENT_H_
#define SKINMATRIXPROVIDERCOMPONENT_H_
#include <vector>
#include "Component.h"

struct Skeleton;
class AnimatorComponent;
class BoneManager;
class TransformComponent;
class Matrix4x4;

class SkinMatrixProviderComponent : public Component
{
public:
	enum class Mode
	{
		Animation, // Animation の SkinMatrix をそのまま使う
		Ragdoll,   // 物理（BoneManager）から作った SkinMatrix を使う
		Hybrid,	   // 一部物理、一部アニメーション
	};

private:
	// ==================================================
	// ----- 構成要素 -----
	// ==================================================
	Skeleton*			m_pSkeleton = nullptr;
	AnimatorComponent*	m_pAnimator = nullptr;
	BoneManager*		m_pBoneManager = nullptr;
	TransformComponent* m_pOwnerTransform = nullptr;

	// ==================================================
	// ----- モード -----
	// ==================================================
	Mode m_Mode = Mode::Animation;

	// ==================================================
	// ----- マトリクス -----
	// ==================================================
	// 最終姿勢を作るときに、この Bone は Physics を採用するかどうか
	std::vector<char> m_UsePhysicsBone; // 0 = Animation, 1 = Physics
	std::vector<Matrix4x4> m_SkinPhys; // 物理由来の Skin
	std::vector<Matrix4x4> m_SkinOut;  // ブレンド後の最終出力用の Skin（現在未使用）
	std::vector<Matrix4x4> m_FinalModel;  // 最終モデル姿勢（ボーンごと）

public:
	// ==================================================
	// ----- 初期セットアップ（必ず呼ぼうね） -----
	// ==================================================
	void SetUp(Skeleton* skeleton, AnimatorComponent* anime, BoneManager* boneManager, TransformComponent* ownerTrans);

	// ==================================================
	// ----- BoneObject の Physics 設定 -----
	// これをしないとアニメで上書きされてしまう
	// 格納された場合、Physics 優先なので Transform 側からの処理は上書きされてしまう
	// ==================================================
	void SetBonePhysics(int boneIndex, bool enable) { m_UsePhysicsBone[boneIndex] = enable ? 1 : 0; }

	// ==================================================
	// ----- モード関係 -----
	// ==================================================
	void SetMode(Mode m) { m_Mode = m; }
	void SetModeAnimation() { m_Mode = Mode::Animation; }
	void SetModeRagdoll()	{ m_Mode = Mode::Ragdoll; }
	void SetModeHybrid()	{ m_Mode = Mode::Hybrid; }
	Mode GetMode() const { return m_Mode; }
	bool IsAniamtion() const { return m_Mode == Mode::Animation; }
	bool IsRagdoll()   const { return m_Mode == Mode::Ragdoll; }
	bool IsHybrid()	   const { return m_Mode == Mode::Hybrid; }

	// ==================================================
	// ----- アクセサ -----
	// ==================================================
	const std::vector<char>& UsePhysicsBoneMask() const { return m_UsePhysicsBone; }

	// ==================================================
	// ----- Draw 側がこれを呼ぶ -----
	// ==================================================
	const Matrix4x4* GetSkinMatrixPtr(int& outCount);
private:
	// BoneManager からボーンの行列を作成
	void BuildPhysicsSkinMatrixes();
	// Hybrid なボーンの行列を作成
	void BuildHybridSkinMatrixes();
};


#endif