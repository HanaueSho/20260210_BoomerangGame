/*
	Camera.h
	20250818 hanaue sho
*/
#ifndef CAMERA_H_
#define CAMERA_H_
#include "GameObject.h"
#include "TransformComponent.h"
#include "CameraComponent.h"

// ----- MyCameraClass -----
// --------------------------------------------------
// カメラ基底クラス
// --------------------------------------------------
class Camera : public GameObject
{
public:
	void Init() override;
	//void Uninit() override {}
	void Update(float dt) override;
	//void Draw() override {}
};


/*
// --------------------------------------------------
// 自由カメラ
// ターゲットに追従するカメラ
// --------------------------------------------------
class FreeCamera : public Camera
{
private:
	XMVECTOR m_Position = {}; // 座標
	XMFLOAT3 m_TargetPosition = {};
	float m_Distance = -5.0f; // 対象までの水平距離

	XMVECTOR m_VectorRight = { 1.0f, 0.0f, 0.0f }; // 右方向ベクトル
	XMVECTOR m_VectorTop = { 0.0f, 1.0f, 0.0f }; // 上方向ベクトル
	XMVECTOR m_VectorFront = { 0.0f, 0.0f, 1.0f }; // 前方向ベクトル

	XMVECTOR m_FocusPosition = {}; // 注視点

public:
	FreeCamera() = delete;
	FreeCamera(const XMFLOAT3& targetPosition, float dist) : m_TargetPosition(targetPosition), m_Distance(dist) {}

	void Init() override {}
	void Uninit() override {}
	void Update() override;
	void Draw() override;


	XMVECTOR GetPosition() const { return m_Position; }
	XMVECTOR GetVectorRight() const { return m_VectorRight; }
	XMVECTOR GetVectorTop() const { return m_VectorTop; }
	XMVECTOR GetVectorFront() const { return m_VectorFront; }
};

// --------------------------------------------------
// フォローカメラ
// ターゲットに追従するカメラ
// --------------------------------------------------
class FollowCamera : public Camera
{
private:
	XMVECTOR m_Position = {0, 0, 0}; // 座標
	GameObject* m_pTargetObject = nullptr;
	float m_Distance = -5.0f; // 対象までの水平距離

	XMVECTOR m_VectorRight = { 1.0f, 0.0f, 0.0f }; // 右方向ベクトル
	XMVECTOR m_VectorTop = { 0.0f, 1.0f, 0.0f }; // 上方向ベクトル
	XMVECTOR m_VectorFront = { 0.0f, 0.0f, 1.0f }; // 前方向ベクトル

	XMVECTOR m_FocusPosition = {0, 0, 1}; // 注視点

public:
	FollowCamera() = default;
	FollowCamera(GameObject* pTarget, float dist) : m_pTargetObject(pTarget), m_Distance(dist) {}

	void Init() override {}
	void Uninit() override {}
	void Update();
	void Draw() override;

	XMVECTOR GetPosition() const { return m_Position; }
	XMVECTOR GetVectorRight() const { return m_VectorRight; }
	XMVECTOR GetVectorTop() const { return m_VectorTop; }
	XMVECTOR GetVectorFront() const { return m_VectorFront; }

	void SetTarget(GameObject* pObject) { m_pTargetObject = pObject; }
};
*/

#endif