/*
	collision.h
	hanaue sho 20241119
*/
#ifndef COLLISION_H_
#define COLLISION_H_
#include "Vector3.h"

// ==================================================
// ----- 構造体 -----
// ==================================================
struct AABB
{
	Vector3 min{}, max{};
	bool isOverlap(const AABB& o) const
	{
		return !(max.x < o.min.x || min.x > o.max.x
			  || max.y < o.min.y || min.y > o.max.y
			  || max.z < o.min.z || min.z > o.max.z);
	}
};

// ==================================================
// ----- 前方宣言 -----
// ==================================================
class GameObject;
class TransformComponent; 
struct ContactManifold; // コイツはクラスじゃないので注意
class BoxCollision; 
class SphereCollision; 
class CapsuleCollision;
class HeightMapCollision;
struct Triangle;
struct ColliderPose;

// ==================================================
// ----- ３Dコリジョンインタフェース -----
// ==================================================
class Collision
{
public:
	virtual ~Collision() = default;

	// --------------------------------------------------
	// ブロード用
    // --------------------------------------------------
	virtual AABB ComputeWorldAABB(const ColliderPose& ownerTrans) const = 0;

	// --------------------------------------------------
	// ダブルディスパッチ方式
    // --------------------------------------------------
	virtual bool isOverlap			   (const ColliderPose& myTrans, const Collision& collisionB,	    const ColliderPose& transB  , ContactManifold& out, float slop) const = 0;
	virtual bool isOverlapWithBox	   (const ColliderPose& myTrans, const BoxCollision& box,		    const ColliderPose& transBox, ContactManifold& out, float slop) const = 0;
	virtual bool isOverlapWithSphere   (const ColliderPose& myTrans, const SphereCollision& sphere,	const ColliderPose& transSph, ContactManifold& out, float slop) const = 0;
	virtual bool isOverlapWithCapsule  (const ColliderPose& myTrans, const CapsuleCollision& capsule, const ColliderPose& transCap, ContactManifold& out, float slop) const = 0;
	virtual bool isOverlapWithHeightMap(const ColliderPose& myTrans, const HeightMapCollision& map,   const ColliderPose& transMap, ContactManifold& out, float slop) const = 0;
};

// ==================================================
// ----- Box -----
// ==================================================
class BoxCollision : public Collision
{
private:
	Vector3 m_HalfSize; // 半分のサイズ

public:
	BoxCollision(const Vector3& halfSize) : m_HalfSize(halfSize) {}

	const Vector3& HalfSize() const { return m_HalfSize; }

	AABB ComputeWorldAABB(const ColliderPose& ownerTrans) const override;

	bool isOverlap			 (const ColliderPose& myTrans, const Collision& collisionB,	  const ColliderPose& transB  , ContactManifold& out, float slop) const override;
	bool isOverlapWithBox	 (const ColliderPose& myTrans, const BoxCollision& box,		  const ColliderPose& transBox, ContactManifold& out, float slop) const override;
	bool isOverlapWithSphere (const ColliderPose& myTrans, const SphereCollision& sphere,	  const ColliderPose& transSph, ContactManifold& out, float slop) const override;
	bool isOverlapWithCapsule(const ColliderPose& myTrans, const CapsuleCollision& capsule, const ColliderPose& transCap, ContactManifold& out, float slop) const override;
	bool isOverlapWithHeightMap(const ColliderPose& myTrans, const HeightMapCollision& map, const ColliderPose& transMap, ContactManifold& out, float slop) const override;
};

// ==================================================
// ----- Sphere -----
// ==================================================
class SphereCollision : public Collision
{
private:
	float m_Radius; // 半径

public:
	SphereCollision(float radius) : m_Radius(radius) {}

	float Radius() const { return m_Radius; }

	AABB ComputeWorldAABB(const ColliderPose& ownerTrans) const override;

	bool isOverlap			 (const ColliderPose& myTrans, const Collision& collisionB,	  const ColliderPose& transB  , ContactManifold& out, float slop) const override;
	bool isOverlapWithBox	 (const ColliderPose& myTrans, const BoxCollision& box,		  const ColliderPose& transBox, ContactManifold& out, float slop) const override;
	bool isOverlapWithSphere (const ColliderPose& myTrans, const SphereCollision& sphere,	  const ColliderPose& transSph, ContactManifold& out, float slop) const override;
	bool isOverlapWithCapsule(const ColliderPose& myTrans, const CapsuleCollision& capsule, const ColliderPose& transCap, ContactManifold& out, float slop) const override;
	bool isOverlapWithHeightMap(const ColliderPose& myTrans, const HeightMapCollision& map, const ColliderPose& transMap, ContactManifold& out, float slop) const override;
};

// ==================================================
// ----- Capsule ----- 
// ==================================================
class CapsuleCollision : public Collision
{
private:
	float m_Radius = 1.0f; // 半径
	float m_CylinderHeight = 1.0f; // 高さ

public:
	CapsuleCollision(float radius, float cylinderHeight) : m_Radius(radius), m_CylinderHeight(cylinderHeight) {}

	float Radius() const { return m_Radius; }
	float CylinderHeight() const { return m_CylinderHeight; }

	AABB ComputeWorldAABB(const ColliderPose& ownerTrans) const override;
	
	bool isOverlap			 (const ColliderPose& myTrans, const Collision& collisionB,	  const ColliderPose& transB  , ContactManifold& out, float slop) const override;
	bool isOverlapWithBox	 (const ColliderPose& myTrans, const BoxCollision& box,		  const ColliderPose& transBox, ContactManifold& out, float slop) const override;
	bool isOverlapWithSphere (const ColliderPose& myTrans, const SphereCollision& sphere,	  const ColliderPose& transSph, ContactManifold& out, float slop) const override;
	bool isOverlapWithCapsule(const ColliderPose& myTrans, const CapsuleCollision& capsule, const ColliderPose& transCap, ContactManifold& out, float slop) const override;
	bool isOverlapWithHeightMap(const ColliderPose& myTrans, const HeightMapCollision& map, const ColliderPose& transMap, ContactManifold& out, float slop) const override;
};

// ==================================================
// ----- HeightMap ----- 
// ==================================================
class HeightMapCollision : public Collision
{
private:
	int m_Width = 100; // マップサイズ
	int m_Depth = 100; // マップサイズ
	float m_CellSizeX = 1.0f; // セルサイズ
	float m_CellSizeZ = 1.0f; // セルサイズ
	std::vector<float> m_Heights; // 高さマップ

	// キャッシュ
	float m_MinHeight = 0.0f;
	float m_MaxHeight = 0.0f;
	// 対応する Triangle の計算
	bool BuildCellTrianglesWorld(int i, int j, const ColliderPose& myTrans, Triangle& out0, Triangle& out1) const;
public:
	HeightMapCollision(int width, int depth, float cellSizeX, float cellSizeZ, std::vector<float> heights) : m_Width(width), m_Depth(depth), m_CellSizeX(cellSizeX), m_CellSizeZ(cellSizeZ), m_Heights(heights) 
	{
		assert(width > 0 && depth > 0);
		assert(static_cast<int>(heights.size()) == width * depth);

		float min = m_Heights[0];
		float max = m_Heights[0];
		for (int i = 0; i < m_Heights.size(); i++)
		{
			float height = m_Heights[i];
			if (min > height) min = height;
			if (max < height) max = height;
		}
		m_MinHeight = min;
		m_MaxHeight = max;
	}

	float Width() const { return m_Width; }
	float Depth() const { return m_Depth; }
	float CellSizeX() const { return m_CellSizeX; }
	float CellSizeZ() const { return m_CellSizeZ; }
	const std::vector<float>& Heights() const { return m_Heights; };
	float GetHeight(int x, int z) const { return m_Heights[z * m_Width + x]; }

	AABB ComputeWorldAABB(const ColliderPose& ownerTrans) const override;

	bool isOverlap			 (const ColliderPose& myTrans, const Collision& collisionB,	  const ColliderPose& transB,   ContactManifold& out, float slop) const override;
	bool isOverlapWithBox	 (const ColliderPose& myTrans, const BoxCollision& box,		  const ColliderPose& transBox, ContactManifold& out, float slop) const override;
	bool isOverlapWithSphere (const ColliderPose& myTrans, const SphereCollision& sphere,	  const ColliderPose& transSph, ContactManifold& out, float slop) const override;
	bool isOverlapWithCapsule(const ColliderPose& myTrans, const CapsuleCollision& capsule, const ColliderPose& transCap, ContactManifold& out, float slop) const override;
	bool isOverlapWithHeightMap(const ColliderPose& myTrans, const HeightMapCollision& map, const ColliderPose& transMap, ContactManifold& out, float slop) const override;
};

#endif