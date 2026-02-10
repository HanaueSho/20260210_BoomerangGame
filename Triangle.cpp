#include "Triangle.h"

// 点ｐに最も近い三角形上の点の算出
static Vector3 ClosestPointOnTriangle(const Vector3& p, const Triangle& tri)
{
    // Triangle の３頂点座標
    const Vector3& a = tri.a;
    const Vector3& b = tri.b;
    const Vector3& c = tri.c;

    Vector3 ab = b - a;
    Vector3 ac = c - a;
    Vector3 ap = p - a;

    float d1 = Vector3::Dot(ab, ap);
    float d2 = Vector3::Dot(ac, ap);

    // 頂点 A 領域
    if (d1 <= 0.0f && d2 <= 0.0f)
        return a;

    // 頂点 B 領域
    Vector3 bp = p - b;
    float d3 = Vector3::Dot(ab, bp);
    float d4 = Vector3::Dot(ac, bp);
    if (d3 >= 0.0f && d4 <= d3)
        return b;

    // 頂点 C 領域
    Vector3 cp = p - c;
    float d5 = Vector3::Dot(ab, cp);
    float d6 = Vector3::Dot(ac, cp);
    if (d6 >= 0.0f && d5 <= d6)
        return c;

    // 辺 AB 領域
    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
    {
        float v = d1 / (d1 - d3);
        return a + ab * v;
    }

    // 辺 AC 領域
    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
    {
        float w = d2 / (d2 - d6);
        return a + ac * w;
    }

    // 辺 BC 領域
    float va = d3 * d6 - d5 * d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
    {
        Vector3 bc = c - b;
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + bc * w;
    }

    // 面内部領域　→　三角形面への射影
    Vector3 n = Vector3::Cross(ab, ac).normalized();
    float dist = Vector3::Dot(p - a, n);
    return p - n * dist;
}

bool CollisionTriangle::IntersectBox()
{
    return false;
}

bool CollisionTriangle::IntersectSphere(const Vector3& center, float radius, const Triangle& tri, TriHit& outHit)
{
    // 球の中心に最も近い三角形上の点を求める
    Vector3 closest = ClosestPointOnTriangle(center, tri);

    // 距離との差分ベクトル
    Vector3 diff = closest - center;
    float distSq = diff.lengthSq();
    float r = radius;

    // 半径の２乗より大きいなら当たっていない
    if (distSq > r * r)
    {
        outHit.hit = false;
        return false;
    }

    float dist = diff.length();

    // penetration depth
    float penetration = r - dist;

    // 法線を求める
    Vector3 normal = { 1, 0, 0 };
    if (dist > 1e-8f)
    {
        // 球の中心 → 最近接点の方向
        normal = diff / dist; // Sphere → Triangle
    }
    else
    {
        // center が三角形上ほぼ直上にある場合：三角形の法線を使う
        Vector3 ab = tri.b - tri.a;
        Vector3 ac = tri.c - tri.a;
        normal = -Vector3::Cross(ab, ac).normalized(); // Sphere → Tri に向けるために－を付ける
    }

    // 接触点
    Vector3 contactPointOnSphere   = center + normal * radius; // sphere 上の点
    Vector3 contactPointOnTriangle = closest; // triangle 上の点

    outHit.hit = true;
    outHit.penetration = penetration;
    outHit.normal = normal;
    outHit.pointOnA = contactPointOnSphere;
    outHit.pointOnB = contactPointOnTriangle;

    return true;
}

bool CollisionTriangle::IntersectCapsule()
{
    return false;
}
