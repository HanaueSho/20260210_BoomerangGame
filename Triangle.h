/*
	Triangle.h
	20251208  hanaue sho
*/
#ifndef TRIANGLE_H_
#define TRIANGLE_H_
#include "Vector3.h"

struct Triangle
{
	Vector3 a, b, c; // Triangle を構成する３頂点（ワールド座標）
};

struct TriHit
{
	bool hit = false;
	float penetration = 0.0f;
	Vector3 normal{};
	Vector3 pointOnA{};
	Vector3 pointOnB{};
};

namespace CollisionTriangle
{
	bool IntersectBox();
	bool IntersectSphere(const Vector3& center, float radius, const Triangle& tri, TriHit& outHit);
	bool IntersectCapsule();
}

#endif