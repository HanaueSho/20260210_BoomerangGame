/*
	MeshFactory.h
	20250818  hanaue sho
	自作メッシュを作る
	作ったらそのまま MeshFilter にセットする（所有権は MeshFilter ）
*/
#ifndef MESHFACTORY_H_
#define MESHFACTORY_H_
#include <d3d11.h>
#include <cassert>
#include "MeshFilterComponent.h"
#include "Renderer.h"
#include "Vector3.h"


// ==================================================
// ----- パラメータ -----
// ==================================================
// --------------------------------------------------
// 平面（XY）
// 第１引数：width
// 第２引数：height
// 第３引数：originCenter
// --------------------------------------------------
struct QuadParams
{
	float width = 200.0f;
	float height = 200.0f;
	bool originCenter = true; // true = 中心原点, false = 左上原点
};
// --------------------------------------------------
// 平面（XZ）
// 第１引数：width
// 第２引数：depth
// 第３引数：originCenter
// --------------------------------------------------
struct PlaneParams
{
	float width = 10.0f;
	float depth = 10.0f;
	bool originCenter = true; // true = 中心原点, false = 左上原点
};
// --------------------------------------------------
// 球体
// 第１引数：radius
// 第２引数：slices
// 第３引数：stacks
// 第４引数：insideOut
// --------------------------------------------------
struct SphereParams
{
	float radius = 1.0f;
	int slices = 6; // 横分割（経度）
	int stacks = 6; // 縦分割（緯度）
	bool insideOut = false; // true: スカイドーム用
};
// --------------------------------------------------
// 立方体
// 第１引数：size
// 第２引数：originCenter
// 第３引数：insideOut
// --------------------------------------------------
struct CubeParams
{
	Vector3 size = Vector3(1, 1, 1);
	bool originCenter = true;   // true = 中心原点, false = 左上原点
	bool insideOut = false;		// true: 内側
};
// --------------------------------------------------
// カプセル
// 第１引数：radius
// 第２引数：cylinderHeight
// 第３引数：slices
// 第４引数：stacksBody
// 第５引数：stacksCap
// 第６引数：insideOut
// --------------------------------------------------
struct CapsuleParams
{
	float radius = 1.0f;		 // 半径
	float cylinderHeight = 1.0f; // 胴の長さ
	int slices = 24;			 // 周方向
	int stacksBody = 1;			 // 側面の縦分割
	int stacksCap  = 4;			 // 半球の縦分割
	bool insideOut = false;		 // 反転
};
// --------------------------------------------------
// 円柱
// 第１引数：radius
// 第２引数：height
// 第３引数：slices
// 第４引数：stacks
// 第５引数：insideOut
// --------------------------------------------------
struct CylinderParams
{
	float radius = 1.0f;
	float height = 2.0f;
	int slices = 24;		// 周方向
	int stacks = 1;			// 側面の縦分割
	bool insideOut = false;	// 反転
};
// --------------------------------------------------
// メッシュフィールド
// 第１引数：width
// 第２引数：depth
// 第３引数：cellSizeX
// 第４引数：cellSizeZ
// 第５引数：heights
// 第６引数：textureSpread
// --------------------------------------------------
struct MeshFieldParams
{
	int width = 100; // マップサイズ
	int depth = 100; // マップサイズ
	float cellSizeX = 1.0f; // セルサイズ
	float cellSizeZ = 1.0f; // セルサイズ
	std::vector<float> heights; // 高さマップ
	bool textureSpread = false; // テクスチャを敷き詰めるか
};
// --------------------------------------------------
// リンゴ
// 第１引数：radius
// 第２引数：slices
// 第３引数：stacks
// 第４引数：insideOut
// --------------------------------------------------
struct AppleParams
{
	float radius = 1.0f;
	int slices = 6; // 横分割（経度）
	int stacks = 6; // 縦分割（緯度）
	bool insideOut = false; // true: 内側
};


// ==================================================
// ----- クラス -----
// ==================================================
class MeshFactory
{
public:
	// --------------------------------------------------
	// 平面（XY）
	// 第１引数：width
	// 第２引数：height
	// 第３引数：originCenter
	// --------------------------------------------------
	static void CreateQuad	   (MeshFilterComponent* filter, const QuadParams& p = {});
	// --------------------------------------------------
	// 平面（XZ）
	// 第１引数：width
	// 第２引数：depth
	// 第３引数：originCenter
	// --------------------------------------------------
	static void CreatePlane    (MeshFilterComponent* filter, const PlaneParams& p = {});
	// --------------------------------------------------
	// 球体
	// 第１引数：radius
	// 第２引数：slices
	// 第３引数：stacks
	// 第４引数：insideOut
	// --------------------------------------------------
	static void CreateSphere   (MeshFilterComponent* filter, const SphereParams& p = {});
	// --------------------------------------------------
	// 立方体
	// 第１引数：size
	// 第２引数：originCenter
	// 第３引数：insideOut
	// --------------------------------------------------
	static void CreateCube	   (MeshFilterComponent* filter, const CubeParams& p = {});
	// --------------------------------------------------
	// 円柱
	// 第１引数：radius
	// 第２引数：height
	// 第３引数：slices
	// 第４引数：stacks
	// 第５引数：insideOut
	// --------------------------------------------------
	static void CreateCylinder (MeshFilterComponent* filter, const CylinderParams& p = {});
	// --------------------------------------------------
	// カプセル
	// 第１引数：radius
	// 第２引数：cylinderHeight
	// 第３引数：slices
	// 第４引数：stacksBody
	// 第５引数：stacksCap
	// 第６引数：insideOut
	// --------------------------------------------------
	static void CreateCapsule  (MeshFilterComponent* filter, const CapsuleParams& p = {});
	// --------------------------------------------------
	// メッシュフィールド
	// 第１引数：width
	// 第２引数：depth
	// 第３引数：cellSizeX
	// 第４引数：cellSizeZ
	// 第５引数：heights
	// 第６引数：textureSpread
	// --------------------------------------------------
	static void CreateMeshField(MeshFilterComponent* filter, const MeshFieldParams& p = {});
	// --------------------------------------------------
	// リンゴ
	// 第１引数：radius
	// 第２引数：slices
	// 第３引数：stacks
	// 第４引数：insideOut
	// --------------------------------------------------
	static void CreateApple	   (MeshFilterComponent* filter, const AppleParams& p = {});
};



#endif