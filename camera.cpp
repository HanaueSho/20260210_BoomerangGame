/*
	Camera.cpp
	20250818 hanaue sho
*/
#include "Camera.h"
#include "CameraMove.h"
#include "Mouse.h"
#include "Main.h" // SCREEN_SIZE用
#include "Keyboard.h"

// --------------------------------------------------
// カメラ（基底クラス）
// --------------------------------------------------
void Camera::Init()
{
	auto* tf = GetComponent<TransformComponent>();
	auto* camera = AddComponent<CameraComponent>();
	auto* cameraMove = AddComponent<CameraMove>();

	// 2D設定
	camera->SetMode(CameraComponent::Mode::Ortho2D);
	camera->SetOrthoOffCenter(0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, 0.0f, 0.1f);
	camera->UseTransformIn2D(false); // 固定 2D カメラ

	// 3D設定
	// cam->SetMode(CameraComponent::Mode::Perspective);
	// cam->SetPerspective(DirectX::XMConvertToRadians(60.0f),
	//                     (float)SCREEN_WIDTH / SCREEN_HEIGHT,
	//                     0.1f, 1000.0f);
	// tf->SetPosition({0, 2, -5});
	

}
void Camera::Update(float dt) 
{
	GameObject::Update(dt);

	// ----- カメラ制御 -----
	//if (Keyboard_IsKeyDownTrigger(KK_D7)) // 初期位置
	//{
	//	Transform()->SetPosition({ 0, 10, -15 });
	//	Transform()->SetEulerAngles({ 0.5, 0, 0 });
	//}
	//if (Keyboard_IsKeyDownTrigger(KK_D1)) // メッシュフィールド
	//{
	//	Transform()->SetPosition({ 3, 5, -10 });
	//	Transform()->SetEulerAngles({ 0.8, -0.5, 0 });
	//}
	//if (Keyboard_IsKeyDownTrigger(KK_D2)) // 回転床
	//{
	//	Transform()->SetPosition({ 0, 10, 15 });
	//	Transform()->SetEulerAngles({ 0.5, -0.2, 0 });
	//}
	//if (Keyboard_IsKeyDownTrigger(KK_D3)) // 坂道
	//{
	//	Transform()->SetPosition({ 30, 30, 10 });
	//	Transform()->SetEulerAngles({ 0.5, -0.2, 0 });
	//}
	//if (Keyboard_IsKeyDownTrigger(KK_D4)) // キャラ
	//{
	//	Transform()->SetPosition({ 50, 15, -15 });
	//	Transform()->SetEulerAngles({ 0.5, 0, 0 });
	//}
	//if (Keyboard_IsKeyDownTrigger(KK_D5)) // 独楽
	//{
	//	Transform()->SetPosition({ 20, 10, -55 });
	//	Transform()->SetEulerAngles({ 0.5, 0, 0 });
	//}
	//if (Keyboard_IsKeyDownTrigger(KK_D6)) // バウンド
	//{
	//	Transform()->SetPosition({ -45, 20, -30 });
	//	Transform()->SetEulerAngles({ 0.5, 0, 0 });
	//}
}


/*
// --------------------------------------------------
// スクリーン上の座標を返す
// 引数：ワールド座標
// --------------------------------------------------
XMFLOAT2 Camera::GetPositionOnScreen(const XMFLOAT3& position)
{
	XMMATRIX worldMatrix = XMMatrixTranslation(position.x, position.y, position.z); // モデル座標からワールド座標への変換
	XMMATRIX viewMatrix = m_mtxView; // ワールド座標からビュー座標への変換
	XMMATRIX projectionMatrix = m_mtxPerspective; // ビュー座標からプロジェクション座標への変換

	XMVECTOR worldPos = { position.x, position.y, position.z, 1.0f }; // オブジェクトの座標（ワールド座標）
	XMVECTOR viewPos = XMVector3TransformCoord(worldPos, viewMatrix); // ビュー空間へ変換
	XMVECTOR projPos = XMVector3TransformCoord(viewPos, projectionMatrix); // プロジェクション空間へ変換

	// プロジェクション行列の結果として得られる座標は[-1, 1]の範囲
	XMFLOAT2 positionOnScreen;
	positionOnScreen.x = (projPos.m128_f32[0] + 1.0f) * 0.5f * SCREEN_WIDTH;
	positionOnScreen.y = (1.0f - projPos.m128_f32[1]) * 0.5f * SCREEN_HEIGHT;

	/*
		備忘録
		特定の位置関係によってはカメラの背面にあるものが画面内の座標を指してしまうことがある。
		対応策としてはカメラとの位置関係を確認して背面のときは描画しないようにするなど。
	

	return positionOnScreen;
}
*/
/*
// --------------------------------------------------
// フリーカメラ
// 自由に動けるカメラ
// マウスホイール : ターゲットまでの距離調整
// 右クリック : 旋回
// --------------------------------------------------
void FreeCamera::Update()
{
	// マウスによるカメラ移動 ----------
	if (Mouse_IsWheelScroll()) m_Distance += 0.5f * Mouse_GetWheelScroll(); // 距離調整
	m_Distance = Clamp(m_Distance, -70, -0.5f);

	// クリックしている間にマウスの移動量を反映 -----
	static XMFLOAT2 mousePos = {};
	static XMFLOAT2 mousePosOld = {};
	// 右クリックで回転 -----
	if (Mouse_IsClick(MS_CLICK_RIGHT))
	{
		// 取得
		if (Mouse_IsClickTrigger(MS_CLICK_RIGHT))
		{
			mousePosOld.x = Mouse_GetPositionX();
			mousePosOld.y = Mouse_GetPositionY();
		}
		mousePos.x = Mouse_GetPositionX();
		mousePos.y = Mouse_GetPositionY();

		// ローカルベクトルを回転させる
		{
			XMMATRIX rotation = XMMatrixRotationAxis(m_VectorTop, -(mousePosOld.x - mousePos.x) * 0.005f);
			m_VectorFront = XMVector3TransformNormal(m_VectorFront, rotation);
			m_VectorRight = XMVector3Cross(m_VectorTop, m_VectorFront); // 外積

			m_VectorFront = XMVector3Normalize(m_VectorFront); // 正規化
			m_VectorRight = XMVector3Normalize(m_VectorRight); // 正規化
		}
		{
			XMMATRIX rotation = XMMatrixRotationAxis(m_VectorRight, -(mousePosOld.y - mousePos.y) * 0.005f);
			m_VectorFront = XMVector3TransformNormal(m_VectorFront, rotation);
			m_VectorTop = XMVector3Cross(m_VectorFront, m_VectorRight); // 外積

			m_VectorFront = XMVector3Normalize(m_VectorFront); // 正規化
			m_VectorTop = XMVector3Normalize(m_VectorTop); // 正規化
		}

		// 記録
		mousePosOld.x = mousePos.x;
		mousePosOld.y = mousePos.y;
	}
	// 中クリックで移動 -----
	if (Mouse_IsClick(MS_CLICK_MIDDLE))
	{
		if (Mouse_IsClickTrigger(MS_CLICK_MIDDLE))
		{
			mousePosOld.x = Mouse_GetPositionX();
			mousePosOld.y = Mouse_GetPositionY();
		}
		mousePos.x = Mouse_GetPositionX();
		mousePos.y = Mouse_GetPositionY();

		// マウスの移動量で移動 -----
		XMFLOAT3 targetPosition = m_TargetPosition;
		XMFLOAT3 cameraPosition = {};
		XMStoreFloat3(&cameraPosition, m_Position);
		XMFLOAT3 vecTop = {};
		XMStoreFloat3(&vecTop, m_VectorTop);
		XMFLOAT3 vecRight = {};
		XMStoreFloat3(&vecRight, m_VectorRight);

		XMFLOAT2 diff = (mousePosOld - mousePos) * 0.01f;
		diff.y *= -1;

		targetPosition.x += vecTop.x * diff.y + vecRight.x * diff.x;
		targetPosition.y += vecTop.y * diff.y + vecRight.y * diff.x;
		targetPosition.z += vecTop.z * diff.y + vecRight.z * diff.x;
		cameraPosition.x += vecTop.x * diff.y + vecRight.x * diff.x;
		cameraPosition.y += vecTop.y * diff.y + vecRight.y * diff.x;
		cameraPosition.z += vecTop.z * diff.y + vecRight.z * diff.x;

		m_TargetPosition = targetPosition; // 更新
		m_Position = XMLoadFloat3(&cameraPosition); // 更新

		// 記録
		mousePosOld.x = mousePos.x;
		mousePosOld.y = mousePos.y;

		//printf("m_Position: %f, %f, %f\n", cameraPosition.x, cameraPosition.y, cameraPosition.z);
		//printf("m_Position: %f, %f, %f\n", m_TargetPosition.x, m_TargetPosition.y, m_TargetPosition.z);
	}


	// カメラ座標計算 ----------
	XMMATRIX rotation =
	{
		{m_VectorRight},
		{m_VectorTop},
		{m_VectorFront},
		{0.0f,   0.0f,   0.0f,   1.0f}
	}; // ローカルベクトルによる回転行列
	XMMATRIX translation = XMMatrixTranslationFromVector(XMLoadFloat3(&m_TargetPosition)); // 対象の position
	XMMATRIX translation1 = XMMatrixTranslation(0.0f, 0.0f, m_Distance); // 半径分の移動距離

	XMMATRIX world = translation1 * rotation * translation; // 変換行列

	XMVECTOR position = XMVector3TransformCoord({ 0.0f, 0.0f, 0.0f }, world); // 変換後の position

	XMVECTOR vect = position - m_Position; // 方向ベクトル
	vect *= 0.05f;
	m_Position += vect;

	// カメラの向きの計算 ----------
	XMVECTOR targetPos = XMLoadFloat3(&m_TargetPosition);
	vect = targetPos - m_FocusPosition; // 方向ベクトル
	vect *= 0.05f * 2;
	m_FocusPosition += vect;

	//                               cameraPosition          cameraの向き        cameraの上ベクトル
	XMMATRIX view = XMMatrixLookAtLH(m_Position, m_FocusPosition, m_VectorTop); // LH...LeftHand(左手座標系)
	//                                              cameraの画角  アスペクト比  nearZ  farZ
	XMMATRIX perspective = XMMatrixPerspectiveFovLH(XM_PI / 3.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.01f, 1000.0f); // パースペクティブ座標変換 // XMConvertToRadians(60) でも可

	SetMatrixView(view);
	SetMatrixPerspective(perspective);


}

void FreeCamera::Draw()
{
	//プロジェクション行列を作成
	XMMATRIX	ProjectionMatrix = GetMatrixPerspective();
	//プロジェクション行列をセット
	Renderer::SetProjectionMatrix(ProjectionMatrix);

	//カメラ行列を作成
	XMVECTOR	eyev = m_FocusPosition;
	XMVECTOR	pos = m_Position;
	XMVECTOR	up = m_VectorTop;
	XMMATRIX	ViewMatrix = XMMatrixLookAtLH(pos, eyev, up);

	//カメラ行列をセット
	Renderer::SetViewMatrix(GetMatrixView());

	Renderer::SetDepthEnable(true);

	////カメラ座標セット
	//XMFLOAT3 position;
	//XMStoreFloat3(&position, GetPosition());
	//Renderer::SetCameraPosition(position);
}

// --------------------------------------------------
// フォローカメラ
// ターゲットに追従するカメラ
// マウスホイール : ターゲットまでの距離調整
// 右クリック : 旋回
// --------------------------------------------------
void FollowCamera::Update()
{
	// マウスによるカメラ移動 ----------
	if (Mouse_IsWheelScroll()) m_Distance += 1.5f * Mouse_GetWheelScroll(); // 距離調整
	m_Distance = Clamp(m_Distance, -70, -3);

	// クリックしている間にマウスの移動量を反映
	static XMFLOAT2 mousePos = {};
	static XMFLOAT2 mousePosOld = {};
	if (Mouse_IsClick(MS_CLICK_RIGHT))
	{
		// 取得
		if (Mouse_IsClickTrigger(MS_CLICK_RIGHT))
		{
			mousePosOld.x = Mouse_GetPositionX();
			mousePosOld.y = Mouse_GetPositionY();
		}
		mousePos.x = Mouse_GetPositionX();
		mousePos.y = Mouse_GetPositionY();

		// ローカルベクトルを回転させる
		{
			XMMATRIX rotation = XMMatrixRotationAxis(m_VectorTop, -(mousePosOld.x - mousePos.x) * 0.005f);
			m_VectorFront = XMVector3TransformNormal(m_VectorFront, rotation);
			m_VectorRight = XMVector3Cross(m_VectorTop, m_VectorFront); // 外積

			m_VectorFront = XMVector3Normalize(m_VectorFront); // 正規化
			m_VectorRight = XMVector3Normalize(m_VectorRight); // 正規化
		}
		{
			XMMATRIX rotation = XMMatrixRotationAxis(m_VectorRight, -(mousePosOld.y - mousePos.y) * 0.005f);
			m_VectorFront = XMVector3TransformNormal(m_VectorFront, rotation);
			m_VectorTop = XMVector3Cross(m_VectorFront, m_VectorRight); // 外積

			m_VectorFront = XMVector3Normalize(m_VectorFront); // 正規化
			m_VectorTop = XMVector3Normalize(m_VectorTop); // 正規化
		}

		// 記録
		mousePosOld.x = mousePos.x;
		mousePosOld.y = mousePos.y;
	}

	// カメラ座標計算 ----------
	XMMATRIX rotation =
	{
		{m_VectorRight},
		{m_VectorTop},
		{m_VectorFront},
		{0.0f,   0.0f,   0.0f,   1.0f}
	}; // ローカルベクトルによる回転行列

	XMFLOAT3 positionTarget = { m_pTargetObject->GetPosition().x, m_pTargetObject->GetPosition().y, m_pTargetObject->GetPosition().z };
	XMMATRIX translation = XMMatrixTranslationFromVector(XMLoadFloat3(&positionTarget)); // 対象の position
	XMMATRIX translation1 = XMMatrixTranslation(0.0f, 0.0f, m_Distance); // 半径分の移動距離

	XMMATRIX world = translation1 * rotation * translation; // 変換行列

	XMVECTOR position = XMVector3TransformCoord({ 0.0f, 0.0f, 0.0f }, world); // 変換後の position

	XMVECTOR vect = position - m_Position; // 方向ベクトル
	vect *= 0.05f;
	m_Position += vect;

	// カメラの向きの計算 ----------
	XMVECTOR targetPos = XMLoadFloat3(&positionTarget);
	vect = targetPos - m_FocusPosition; // 方向ベクトル
	vect *= 0.05f;
	m_FocusPosition += vect;

	//                               cameraPosition          cameraの向き        cameraの上ベクトル
	XMMATRIX view = XMMatrixLookAtLH(m_Position, m_FocusPosition, m_VectorTop); // LH...LeftHand(左手座標系)
	//                                              cameraの画角  アスペクト比  nearZ  farZ
	XMMATRIX perspective = XMMatrixPerspectiveFovLH(XM_PI / 3.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.01f, 1000.0f); // パースペクティブ座標変換 // XMConvertToRadians(60) でも可

	SetMatrixView(view);
	SetMatrixPerspective(perspective);

	// わけ合ってむりやり代入（Zソート関係）
	XMFLOAT3 pos;
	Vector3 posVec;
	XMStoreFloat3(&pos, m_Position);
	posVec.x = pos.x;
	posVec.y = pos.y;
	posVec.z = pos.z;
	SetPosition(posVec);
}

void FollowCamera::Draw()
{
	//プロジェクション行列を作成
	XMMATRIX	ProjectionMatrix = GetMatrixPerspective();
	//プロジェクション行列をセット
	Renderer::SetProjectionMatrix(ProjectionMatrix);

	//カメラ行列を作成
	XMVECTOR	eyev = m_FocusPosition;
	XMVECTOR	pos = m_Position;
	XMVECTOR	up = m_VectorTop;
	XMMATRIX	ViewMatrix = XMMatrixLookAtLH(pos, eyev, up);

	//カメラ行列をセット
	Renderer::SetViewMatrix(GetMatrixView());


	Renderer::SetDepthEnable(true);
}
*/
