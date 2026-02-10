/*
	collision.cpp
	hanaue sho 20241119
*/
#include <algorithm>
#include <vector>
#include "Collision.h"
#include "TransformComponent.h"
#include "Matrix4x4.h"
#include "Quaternion.h"
#include "ContactManifold.h"
#include "Triangle.h"
#include "ColliderPose.h"

// ==================================================
// ヘルパ関数
// ==================================================
namespace
{
	// ワールドOBB
	struct OBBW
	{
		Vector3 center;  // center
		Vector3 axis[3]; // world acis
		Vector3 extent;	 // world scale 適用後
	};
	// ワールドOBB の構築
	OBBW MakeOBB(const ColliderPose& pose, const BoxCollision& box)
	{
		OBBW o;
		o.center = pose.position;
		o.axis[0] = pose.WorldRight();
		o.axis[1] = pose.WorldUp();
		o.axis[2] = pose.WorldForward();
		o.extent = { fabsf(pose.scale.x) * box.HalfSize().x,
					 fabsf(pose.scale.y) * box.HalfSize().y,
					 fabsf(pose.scale.z) * box.HalfSize().z };
		return o;
	}

	float AbsDot(const Vector3& a, const Vector3& b) { return fabsf(Vector3::Dot(a, b)); }
	// 最大要素のインデックスを返す
	int ArgMax3(float a, float b, float c) { return (a > b ? (a > c ? 0 : 2) : (b > c ? 1 : 2)); } // {max, return} = {a, 0}, {b, 1}, {c, 2} 

	// 点群を平面でクリップ（Sutherland-Hodgman）
	std::vector<Vector3> ClipPolygonAgainstPlane(const std::vector<Vector3>& poly, const Vector3& n, float d)
	{
		std::vector<Vector3> out;
		if (poly.empty()) return out;
		const int N = (int)poly.size();
		for (int i = 0; i < N; i++)
		{
			const Vector3& A = poly[i]; // 基準頂点（始点）
			const Vector3& B = poly[(i + 1) % N]; // 隣の頂点 （終点）
			const float da = Vector3::Dot(n, A) - d;
			const float db = Vector3::Dot(n, B) - d;
			const bool ina = (da <= 0.0f); // 今回はn方向を“外側”としている
			const bool inb = (db <= 0.0f);
			if (ina && inb)
				out.push_back(B);
			else if (ina && !inb)
			{
				float t = da / (da - db + 1e-20f);
				out.push_back(A + (B - A) * t);
			}
			else if (!ina && inb)
			{
				float t = da / (da - db + 1e-20f);
				out.push_back(A + (B - A) * t);
				out.push_back(B);
			}
		}
		return out;
	}

	// SAT -----
	enum class AxisKind { FaceA, FaceB, EdgeEdge};
	struct AxisChoice 
	{ 
		AxisKind kind;
		int ia; // faceA:0..2 / Edge : A側軸
		int ib; // faceB:0..2 / Edge : B側軸
		float depth; // 最小貫入量
		Vector3 n; // A → B
	}; 
	float SupportRadius(const OBBW& O, const Vector3& n)
	{
		return O.extent.x * AbsDot(O.axis[0], n) + O.extent.y * AbsDot(O.axis[1], n) + O.extent.z * AbsDot(O.axis[2], n);
	}
	bool SAT_AllAxes(const OBBW& A, const OBBW& B, AxisChoice& out, float epsLen2 = 1e-12f)
	{
		const Vector3 vectAtoB = B.center - A.center;
		float best = 1e9f; // 貫入深度
		AxisKind bestKind = AxisKind::FaceA; // 種類
		int bestI = 0, bestJ = 0;  // 
		Vector3 bestN = A.axis[0]; // 

		auto testAxis = [&](const Vector3& nRaw, AxisKind kind, int ia, int ib) -> bool
			{
				float nLen2 = nRaw.lengthSq();
				if (nLen2 < epsLen2) return true; // 無効軸（ほぼ平行）
				Vector3 n = nRaw / sqrtf(nLen2); // 投影軸（分離軸）（正規化）
				float dist = fabsf(Vector3::Dot(vectAtoB, n)); // 投影
				float rA = SupportRadius(A, n); // 半長の投影
				float rB = SupportRadius(B, n); // 半長の投影
				float overlap = rA + rB - dist; // 軸上で比較
				if (overlap < 0.0f) return false; // 分離
				if (overlap < best) { best = overlap; bestKind = kind; bestI = ia; bestJ = ib; bestN = n; }
				return true; // 触れている
			};

		// Face Axis (A, B)
		for (int i = 0; i < 3; i++) if (!testAxis(A.axis[i], AxisKind::FaceA, i, -1)) return false;
		for (int j = 0; j < 3; j++) if (!testAxis(B.axis[j], AxisKind::FaceB, -1, j)) return false;

		// Edge-Edge Axis
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
			{
				Vector3 c = Vector3::Cross(A.axis[i], B.axis[j]);
				if (!testAxis(c, AxisKind::EdgeEdge, i, j)) return false;
			}

		// 法線は A → B に向ける
		if (Vector3::Dot(bestN, vectAtoB) < 0) bestN = -bestN; 

		// AxisChoice の生成
		out.kind = bestKind;
		out.ia = bestI;
		out.ib = bestJ;
		out.depth = best;
		out.n = bestN;

		return true; // 接触
	}

	// ref OBB の refFace を選ぶ
	void BuildRefFaceQuad(const OBBW& R, int refAxis, std::vector<Vector3>& outQuad) 
	{
		static const int others[3][2] = { {1,2}, {0,2}, {0,1} };
		const int u = others[refAxis][0];
		const int v = others[refAxis][1];
		Vector3 n = R.axis[refAxis];
		Vector3 Ru = R.axis[u];
		Vector3 Rv = R.axis[v];
		float eu = R.extent[u];
		float ev = R.extent[v];
		Vector3 center = R.center + n * R.extent[refAxis]; // 表側面
		outQuad.clear();
		outQuad.push_back(center + Ru*eu + Rv*ev); // 右上
		outQuad.push_back(center - Ru*eu + Rv*ev); // 左上
		outQuad.push_back(center - Ru*eu - Rv*ev); // 左下
		outQuad.push_back(center + Ru*eu - Rv*ev); // 右下
	}

	// incidence face : normal に最も反平行な B の面（index を返す）
	int PickIncidenceFace(const OBBW& I, const Vector3& n) 
	{
		float d0 = Vector3::Dot(I.axis[0], n);
		float d1 = Vector3::Dot(I.axis[1], n);
		float d2 = Vector3::Dot(I.axis[2], n);
		return ArgMax3(fabsf(d0), fabsf(d1), fabsf(d2));
	}

	// ２本の線分（中心±dir*Len）最近接点
	void ClosestPointSegmentSegment(
		const Vector3& P0, const Vector3& u, float lu,
		const Vector3& Q0, const Vector3& v, float lv,
		Vector3& outP, Vector3& outQ) 
	{
		Vector3 w0 = P0 - Q0;
		float a = Vector3::Dot(u, u);
		float b = Vector3::Dot(u, v);
		float c = Vector3::Dot(v, v);
		float d = Vector3::Dot(u, w0);
		float e = Vector3::Dot(v, w0);
		float denom = a*c - b*b;
		float s, t;
		if (denom < 1e-12f) // ほぼ平行
		{ 
			s = 0.0f; 
			t = (c > 1e-12f) ? (e / c) : 0.0f; 
		}
		else 
		{ 
			s = (b*e - c*d)/denom; 
			t = (a*e - b*d)/denom; 
		}

		// clamp
		if (s > lu) s = lu; else if (s < -lu) s = -lu;
		if (t > lv) t = lv; else if (t < -lv) t = -lv;
		outP = P0 + u*s;
		outQ = Q0 + v*t;
	}
}


// ==================================================
// BoxCollision
// ==================================================
AABB BoxCollision::ComputeWorldAABB(const ColliderPose& trans) const
{
	Vector3 center = trans.position;
	Matrix4x4 rotate = trans.rotation.ToMatrix();
	Matrix4x4 scale = Matrix4x4::CreateScale(trans.scale.x, trans.scale.y, trans.scale.z);
	Matrix4x4 mat = scale * rotate; // 回転とスケールを持った行列
	Vector3 e; // 各要素の列の絶対値と m_HalfSize の内積
	e.x = fabsf(mat.m[0][0]) * m_HalfSize.x + fabsf(mat.m[1][0]) * m_HalfSize.y + fabsf(mat.m[2][0]) * m_HalfSize.z;
	e.y = fabsf(mat.m[0][1]) * m_HalfSize.x + fabsf(mat.m[1][1]) * m_HalfSize.y + fabsf(mat.m[2][1]) * m_HalfSize.z;
	e.z = fabsf(mat.m[0][2]) * m_HalfSize.x + fabsf(mat.m[1][2]) * m_HalfSize.y + fabsf(mat.m[2][2]) * m_HalfSize.z;

	return { center - e, center + e }; // min: center - e, max: center + e
}
bool BoxCollision::isOverlap(const ColliderPose& myTrans, const Collision& collisionB, const ColliderPose& transB, ContactManifold& out, float slop) const
{
	return collisionB.isOverlapWithBox(transB, *this, myTrans, out, slop);
}
// --------------------------------------------------
// BOX × BOX （SAT方式）
// --------------------------------------------------
bool BoxCollision::isOverlapWithBox(const ColliderPose& myTrans, const BoxCollision& box, const ColliderPose& transBox, ContactManifold& out, float slop) const
{
	// ----- OBB 構築 -----　（ダブルディスパッチの弊害でここはAとB逆に生成する）
	OBBW B = MakeOBB(myTrans, *this);
	OBBW A = MakeOBB(transBox, box);

	// ----- SAT（最小貫入(depth)とその勝ち軸を得る） -----
	AxisChoice ch;
	if (!SAT_AllAxes(A, B, ch)) { out.touching = false; out.count = 0; return false; } // 当たっていなかったら終わり

	// ------ スロープを考慮して接触を判定 ------
	// depth >= -slop なら接触扱い
	const bool touching = (ch.depth >= -slop);
	out.touching = touching;
	if (!touching) { out.count = 0; return false; }

	// 接触法線（SAT で選ばれた軸）
	out.normal = ch.n; // A → B

	// ----- manifold の構築 -----
	out.count = 0;

	if (ch.kind == AxisKind::FaceA || ch.kind == AxisKind::FaceB)
	{
		// face-face
		const bool refIsA = (ch.kind == AxisKind::FaceA); // ReferenceBox の判定
		const OBBW& Ref = refIsA ? A : B; // ReferenceBox の設定
		const OBBW& Inc = refIsA ? B : A; // IncidenceBox の設定
		const int refAxis = refIsA ? ch.ia : ch.ib;

		// 参照面法線は「参照→相手」へ向ける必要がある
		// out.normal は A→B。参照がBのときは反転して B→A にする
		//const Vector3 refN = refIsA ? out.normal : -out.normal;
		const Vector3 refN = refIsA ? out.normal : -out.normal; 

		// 参照面の四角形（今は使ってないので放置）（デバッグ描画に使えるみたい）
		//std::vector<Vector3> refQuad;
		//BuildRefFaceQuad(Ref, refAxis, refQuad);

		// ----- 入射OBB側から、参照法線に最も反平行な面を選ぶ（その４頂点）-----
		const int incFace = PickIncidenceFace(Inc, refN); // incFace の候補
		const int sign = (Vector3::Dot(Inc.axis[incFace], refN) > 0.0f) ? -1.0f : 1.0f; // refNに反平行になるような符号

		// 入射面の４済を作って多角形にする
		std::vector<Vector3> poly; poly.reserve(4);
		{
			// Incidence face の４隅を作る
			const int u = (incFace == 0 ? 1 : 0); // 入射面の２本のエッジ方向（incFace 軸以外の２軸）
			const int v = (incFace == 2 ? 1 : 2);
			
			// n と反平行な「裏側の面」の平面上の中心点 
			const Vector3 c  = Inc.center + Inc.axis[incFace] * Inc.extent[incFace] * sign; // 反対側面（n に反平行）
			const Vector3 U  = Inc.axis[u], V = Inc.axis[v];
			const float   eu = Inc.extent[u], ev = Inc.extent[v];
			poly.push_back(c + U * eu + V * ev); // 右上
			poly.push_back(c - U * eu + V * ev); // 左上
			poly.push_back(c - U * eu - V * ev); // 左下
			poly.push_back(c + U * eu - V * ev); // 右下
		}

		// ----- 参照面の４つの“側面平面”で入射面ポリゴンをクリップ -----
		std::vector<Vector3> poly1 = poly;
		auto clipAgainstSidePlane = [&](const Vector3& pn, float pd) // pn : 平面の法線ベクトル, pd : 原点からのオフセット（平面上の任意の点）
			{
				poly1 = ClipPolygonAgainstPlane(poly1, pn, pd);
			};

		// 参照面上の中心点（表側）
		const int u = (refAxis == 0 ? 1 : 0); // refAxis 以外の２軸
		const int v = (refAxis == 2 ? 1 : 2);
		const Vector3 p0 = Ref.center + refN * Ref.extent[refAxis]; // 参照面上の中心点

		// 左右								// 側面がワールド上のどこにあるか求めている
		clipAgainstSidePlane( Ref.axis[u],  Vector3::Dot(Ref.axis[u], p0) + Ref.extent[u]);
		clipAgainstSidePlane(-Ref.axis[u], -Vector3::Dot(Ref.axis[u], p0) + Ref.extent[u]);
		// 上下
		clipAgainstSidePlane( Ref.axis[v],  Vector3::Dot(Ref.axis[v], p0) + Ref.extent[v]);
		clipAgainstSidePlane(-Ref.axis[v], -Vector3::Dot(Ref.axis[v], p0) + Ref.extent[v]);

		// 残った頂点（接触点候補）を manifold へ
		for (const Vector3& q : poly1)
		{
			if (out.count >= ContactManifold::MAX_POINTS) break; // 上限４つまで（現在は速いもの順）

			// 参照面への有向距離：dot(refN, q - p0)（表側が＋）
			// penetration は参照面への signed 距離の -値を使う（>=0 が重なり）
			const float signedDist = Vector3::Dot(q - p0, refN); // 参照面表側が＋なので <= 0 ならめり込んでいる
			const float pen = -signedDist;					// pen > 0 ならめり込んでいる
			if (pen < -slop) continue; // slop よりも小さい = めり込んでいない

			// A/B それぞれの“自分の面上”の点を作る
			if (ch.kind == AxisKind::FaceA)
			{
				out.points[out.count].pointOnB = q;
				out.points[out.count].pointOnA = q - refN * signedDist; // A 参照面上
			}
			else
			{
				out.points[out.count].pointOnA = q;
				out.points[out.count].pointOnB = q + refN * signedDist; // B 参照面上 
			}
			out.points[out.count].penetration = pen;
			out.count++;
		}

		// クリップで全滅した場合の保険（数値誤差対策）
		if (out.count == 0)
		{
			out.count = 1;
			out.points[0].pointOnA = p0;
			out.points[0].pointOnB = p0;
			out.points[0].penetration = std::max(0.0f, ch.depth);
		}
	} // if (ch.kind == AxisKind::FaceA || ch.kind == AxisKind::FaceB)
	else
	{
		// Edge-Edge（厳密ではなく軽めのもの）
		const int ia = ch.ia, ib = ch.ib;
		// 代表エッジの中心（各OBBの該当軸以外の投影は 0 の中心線でOK）
		const Vector3 Pa0 = A.center; // 代表線分中心
		const Vector3 Qa0 = B.center;
		const Vector3 ua = A.axis[ia]; const float la = A.extent[ia]; // 方向＆半長
		const Vector3 ub = B.axis[ib]; const float lb = B.extent[ib];

		Vector3 pA, pB;
		ClosestPointSegmentSegment(Pa0, ua, la, Qa0, ub, lb, pA, pB);

		out.count = 1;
		out.points[0].pointOnA = pA;
		out.points[0].pointOnB = pB;
		out.points[0].penetration = ch.depth; // SAT の最小貫入をそのままつかう
	}

	return true; // 全ての判定を通ったので true
}
// --------------------------------------------------
// BOX × SPHERE （最近接点方式）
// --------------------------------------------------
bool BoxCollision::isOverlapWithSphere(const ColliderPose& myTrans, const SphereCollision& sphere, const ColliderPose& transSph, ContactManifold& out, float slop) const
{
	// ----- OBBの世界軸を取得 -----
	const Vector3 aX = myTrans.WorldRight();
	const Vector3 aY = myTrans.WorldUp();
	const Vector3 aZ = myTrans.WorldForward();

	// ----- 半エクステント（スケール反映） -----
	Vector3 extent = {
		fabsf(myTrans.scale.x) * m_HalfSize.x,
		fabsf(myTrans.scale.y) * m_HalfSize.y,
		fabsf(myTrans.scale.z) * m_HalfSize.z,
	};

	// ----- 球の中心と半径取得 -----
	const Vector3 centerSph = transSph.position;
	const Vector3 centerBox = myTrans.position;
	const float   radius = sphere.Radius() * fabsf(transSph.scale.x); // 一旦 scale.x でスケーリング

	// ----- 箱中心→球中心へのベクトルを箱の軸に投影 -----
	const Vector3 vect = centerSph - centerBox;
	const Vector3 dist = {
		Vector3::Dot(vect, aX),
		Vector3::Dot(vect, aY),
		Vector3::Dot(vect, aZ)
	};

	// ----- 最近接点のローカル座標（Clamp）-----
	auto Clamp = [](float a, float min, float max)
		{
			float result = a;
			result = std::min(a, max);
			result = std::max(result, min);
			return result;
		};
	float qx = Clamp(dist.x, -extent.x, extent.x);
	float qy = Clamp(dist.y, -extent.y, extent.y);
	float qz = Clamp(dist.z, -extent.z, extent.z);

	// ----- ワールド最近接点 -----
	Vector3 qWorld = centerBox + aX * qx + aY * qy + aZ * qz; // 接触点の計算

	// ----- 球中心　→　最近接点ベクトル ------
	const Vector3 vectSphToPoint = centerSph - qWorld;
	const float distSphToPointSq = vectSphToPoint.lengthSq(); 

	// 球が箱の内部にある特殊ケース対策
	Vector3 n;
	float pen;
	if (distSphToPointSq > 1e-12f) // 球が箱の外にいる
	{
		const float dist = sqrtf(distSphToPointSq);
		n = vectSphToPoint / dist; // 接触点→球　方向（Box → Sphere）
		pen = radius - dist; // 正なら重なっている
	}
	else // 球が箱の内部
	{
		// 最近平面を選ぶ
		const float dx = extent.x - fabsf(dist.x);
		const float dy = extent.y - fabsf(dist.y);
		const float dz = extent.z - fabsf(dist.z);
		if		(dx <= dy && dx <= dz) { n = (dist.x >= 0 ? aX : -aX); qx = (dist.x >= 0 ? extent.x : -extent.x); } // 一番近くの面の方向を normal にする
		else if (dy <= dz)			   { n = (dist.y >= 0 ? aY : -aY); qy = (dist.y >= 0 ? extent.y : -extent.y); }
		else						   { n = (dist.z >= 0 ? aZ : -aZ); qz = (dist.z >= 0 ? extent.z : -extent.z); }

		qWorld = centerBox + aX * qx + aY * qy + aZ * qz; // 接触点の再計算
		pen = radius + std::min(dx, std::min(dy, dz)); // 正で重なっている
	}
	// 接触しているかの判定
	const bool touching = (pen >= -slop);

	// ----- ContactManifold の出力 -----
	out.touching = touching;
	out.normal = -n; // Sphere → Box
	out.count = 1;
	out.points[0].penetration = pen;
	out.points[0].pointOnA = centerSph + out.normal * radius;
	out.points[0].pointOnB = qWorld;

	return touching;
}
// --------------------------------------------------
// BOX × CAPSULE
// --------------------------------------------------
bool BoxCollision::isOverlapWithCapsule(const ColliderPose& myTrans, const CapsuleCollision& capsule, const ColliderPose& transCap, ContactManifold& out, float slop) const
{
	bool b = capsule.isOverlapWithBox(transCap, *this, myTrans, out, slop);
	out.normal = -out.normal;
	return b;
}
// --------------------------------------------------
// BOX × HEIGHTMAP
// --------------------------------------------------
bool BoxCollision::isOverlapWithHeightMap(const ColliderPose& myTrans, const HeightMapCollision& map, const ColliderPose& transMap, ContactManifold& out, float slop) const
{
	bool b = map.isOverlapWithBox(transMap, *this, myTrans, out, slop);
	out.normal = -out.normal;
	return b;
}

// ==================================================
// SphereCollision
// ==================================================
AABB SphereCollision::ComputeWorldAABB(const ColliderPose& trans) const
{
	Vector3 center = trans.position;
	float scaleMax = std::max({ fabsf(trans.scale.x), fabsf(trans.scale.y), fabsf(trans.scale.z)});
	float radius   = m_Radius * scaleMax;
	Vector3 d{ radius, radius, radius };
	return { center - d, center + d }; // min: center - radius, max: center + radius
}
bool SphereCollision::isOverlap(const ColliderPose& myTrans, const Collision& collisionB, const ColliderPose& transB, ContactManifold& out, float slop) const
{
	return collisionB.isOverlapWithSphere(transB, *this, myTrans, out, slop);
}
// --------------------------------------------------
// SPHERE × BOX
// --------------------------------------------------
bool SphereCollision::isOverlapWithBox(const ColliderPose& myTrans, const BoxCollision& box, const ColliderPose& transBox, ContactManifold& out, float slop) const
{
	// ----- OBBの世界軸を取得 -----
	const Vector3 aX = transBox.WorldRight();
	const Vector3 aY = transBox.WorldUp();
	const Vector3 aZ = transBox.WorldForward();

	// ----- 半エクステント（スケール反映） -----
	Vector3 extent = {
		fabsf(transBox.scale.x) * box.HalfSize().x,
		fabsf(transBox.scale.y) * box.HalfSize().y,
		fabsf(transBox.scale.z) * box.HalfSize().z,
	};

	// ----- 球の中心と半径取得 -----
	const Vector3 centerSph = myTrans.position;
	const Vector3 centerBox = transBox.position;
	const float   radius = this->Radius() * fabsf(myTrans.scale.x); // 一旦 scale.x でスケーリング

	// ----- 箱中心→球中心へのベクトルを箱の軸に投影 -----
	const Vector3 vect = centerSph - centerBox;
	const Vector3 dist = {
		Vector3::Dot(vect, aX),
		Vector3::Dot(vect, aY),
		Vector3::Dot(vect, aZ)
	};

	// ----- 最近接点のローカル座標（Clamp）-----
	auto Clamp = [](float a, float min, float max)
		{
			float result = a;
			result = std::min(a, max);
			result = std::max(result, min);
			return result;
		};
	float qx = Clamp(dist.x, -extent.x, extent.x);
	float qy = Clamp(dist.y, -extent.y, extent.y);
	float qz = Clamp(dist.z, -extent.z, extent.z);

	// ----- ワールド最近接点 -----
	Vector3 qWorld = centerBox + aX * qx + aY * qy + aZ * qz; // 接触点の計算

	// ----- 球中心　→　最近接点ベクトル ------
	const Vector3 vectSphToPoint = centerSph - qWorld;
	const float distSphToPointSq = vectSphToPoint.lengthSq();

	// 球が箱の内部にある特殊ケース対策
	Vector3 n;
	float pen;
	if (distSphToPointSq > 1e-12f) // 球が箱の外にいる
	{
		const float dist = sqrtf(distSphToPointSq);
		n = vectSphToPoint / dist; // 接触点→球　方向（Box → Sphere）
		pen = radius - dist; // 正なら重なっている
	}
	else // 球が箱の内部
	{
		// 最近平面を選ぶ
		const float dx = extent.x - fabsf(dist.x);
		const float dy = extent.y - fabsf(dist.y);
		const float dz = extent.z - fabsf(dist.z);
		if (dx <= dy && dx <= dz) { n = (dist.x >= 0 ? aX : -aX); qx = (dist.x >= 0 ? extent.x : -extent.x); } // 一番近くの面の方向を normal にする
		else if (dy <= dz) { n = (dist.y >= 0 ? aY : -aY); qy = (dist.y >= 0 ? extent.y : -extent.y); }
		else { n = (dist.z >= 0 ? aZ : -aZ); qz = (dist.z >= 0 ? extent.z : -extent.z); }

		qWorld = centerBox + aX * qx + aY * qy + aZ * qz; // 接触点の再計算
		pen = radius + std::min(dx, std::min(dy, dz)); // 正で重なっている
	}
	// 接触しているかの判定
	const bool touching = (pen >= -slop);

	// ----- ContactManifold の出力 -----
	out.touching = touching;
	out.normal = n; // Sphere → Box
	out.count = 1;
	out.points[0].penetration = pen;
	out.points[0].pointOnA = centerSph + out.normal * radius;
	out.points[0].pointOnB = qWorld;

	return touching;
}
// --------------------------------------------------
// SPHERE × SPHERE
// --------------------------------------------------
bool SphereCollision::isOverlapWithSphere(const ColliderPose& myTrans, const SphereCollision& sphere, const ColliderPose& transSph, ContactManifold& out, float slop) const
{
	// 中心
	Vector3 centerB = myTrans.position;
	Vector3 centerA = transSph.position;

	// 距離（２乗）
	Vector3 vect = centerB - centerA; // A → B
	float distSq = vect.lengthSq();

	// 半径（スケール反映）
	float radiusA = m_Radius * myTrans.scale.x;			// とりあえずｘの値を使う
	float radiusB = sphere.Radius() * transSph.scale.x; // とりあえずｘの値を使う
	float sum = radiusA + radiusB;

	// 接触判定
	if (distSq > 1e-12f) // ２点間の距離が0以上か確認
	{
		const float d = sqrtf(distSq);
		out.normal = vect / d; // A → B
		out.points[0].penetration = sum - d; // 貫入深度計算
	}
	else
	{
		out.normal = { 1, 0, 0 }; // 退避向き
		out.points[0].penetration = sum;
	}
	out.touching = (out.points[0].penetration >= -slop); // 貫入深度が正 → sum > d → 接触している

	out.points[0].pointOnA = centerA + out.normal * radiusA;
	out.points[0].pointOnB = centerB - out.normal * radiusB;
	out.count = 1;

	return out.touching;
}
// --------------------------------------------------
// SPHERE × CAPSULE（線分と球体の最近接点）
// --------------------------------------------------
bool SphereCollision::isOverlapWithCapsule(const ColliderPose& myTrans, const CapsuleCollision& capsule, const ColliderPose& transCap, ContactManifold& out, float slop) const
{
	bool b = capsule.isOverlapWithSphere(transCap, *this, myTrans, out, slop);
	out.normal = -out.normal;
	return b;
}
// --------------------------------------------------
// SPHERE × HEIGHTMAP
// --------------------------------------------------
bool SphereCollision::isOverlapWithHeightMap(const ColliderPose& myTrans, const HeightMapCollision& map, const ColliderPose& transMap, ContactManifold& out, float slop) const
{
	bool b = map.isOverlapWithSphere(transMap, *this, myTrans, out, slop);
	out.normal = -out.normal;
	return b;
}

// ==================================================
// CapsuleCollision
// ==================================================
AABB CapsuleCollision::ComputeWorldAABB(const ColliderPose& trans) const
{
	Vector3 center = trans.position;
	Vector3 up     = trans.WorldUp(); // ここはワールド上ベクトル？
	Vector3 scale  = trans.scale;

	// ワールド空間での線分の両端
	float halfCylinder = (m_CylinderHeight * scale.y) * 0.5f;
	Vector3 top    = center + up * halfCylinder;
	Vector3 bottom = center - up * halfCylinder;

	// この線分の AABB
	Vector3 minP(
				 std::min(top.x, bottom.x),
				 std::min(top.y, bottom.y),
				 std::min(top.z, bottom.z)
				);
	Vector3 maxP(
				 std::max(top.x, bottom.x),
				 std::max(top.y, bottom.y),
				 std::max(top.z, bottom.z)
				);

	// 半径分だけ全方向に膨らませる
	float radiusScale = std::max(scale.x, scale.z);
	float worldRadius = m_Radius * radiusScale;
	Vector3 e = Vector3(worldRadius, worldRadius, worldRadius);

	return { minP - e , maxP + e };
}
bool CapsuleCollision::isOverlap(const ColliderPose& myTrans, const Collision& collisionB, const ColliderPose& transB, ContactManifold& out, float slop) const
{
	return collisionB.isOverlapWithCapsule(transB, *this, myTrans, out, slop);
}
// --------------------------------------------------
// CAPSULE × BOX（線分とOBBの最近接点）
// --------------------------------------------------
bool CapsuleCollision::isOverlapWithBox(const ColliderPose& myTrans, const BoxCollision& box, const ColliderPose& transBox, ContactManifold& out, float slop) const
{
	// 中心座標
	Vector3 centerBox = transBox.position;
	Vector3 centerCap = myTrans.position;

	// OBB の基底
	Vector3 boxRight   = transBox.WorldRight();
	Vector3 boxUp	   = transBox.WorldUp();
	Vector3 boxForward = transBox.WorldForward();

	Vector3 extent = {
		fabsf(transBox.scale.x) * box.HalfSize().x,
		fabsf(transBox.scale.y) * box.HalfSize().y,
		fabsf(transBox.scale.z) * box.HalfSize().z,
	};

	// カプセルの円柱部の両端座標
	float half = m_CylinderHeight * myTrans.scale.y / 2.0f;
	Vector3 top    = centerCap + half * myTrans.WorldUp();
	Vector3 bottom = centerCap - half * myTrans.WorldUp();

	// 高さ０回避
	if ((top - bottom).lengthSq() < 1e-8f)
	{
		// 球体とみなして isOverlapWithSphere を呼ぶ
		SphereCollision capsule(m_Radius);
		return box.isOverlapWithSphere(transBox, capsule, myTrans, out, slop);
	}

	// カプセルの軸の両端を Box のローカル座標に落とし込む
	Vector3 topOnBoxLocal = {
							 Vector3::Dot(boxRight  , top - centerBox),
							 Vector3::Dot(boxUp     , top - centerBox),
							 Vector3::Dot(boxForward, top - centerBox)
							};
	Vector3 bottomOnBoxLocal = {
								Vector3::Dot(boxRight  , bottom - centerBox),
								Vector3::Dot(boxUp     , bottom - centerBox),
								Vector3::Dot(boxForward, bottom - centerBox)
							   };
	
	// 最近接点に対応する線分上の候補の点
	Vector3 axis = topOnBoxLocal - bottomOnBoxLocal; // bottom → top  
	// p = bottomOnBoxLocal + t * axis より、p = +extent になる t の値 
	auto ComputeT = [&](int index, int sign)
		{
			return  (sign * extent[index] - bottomOnBoxLocal[index]) / axis[index];
		};
	
	std::vector<float> tList;
	tList.push_back(0);
	tList.push_back(1);
	for (int i = 0; i < 6; i++)
	{
		int axisIndex = i / 2;
		if (fabsf(axis[axisIndex]) < 1e-8f) continue; // 軸と平行なので交差しない

		int sign = (i % 2 == 0) ? 1 : -1;
		float t = ComputeT(axisIndex, sign);
		if (t >= 0.0f && t <= 1.0f ) // t = 0..1
			tList.push_back(t);
	}

	// 最近接点を探す（OBB のローカル座標）
	float minDist = FLT_MAX;
	Vector3 pointOnBox{};
	Vector3 pointOnCapsule{};
	float capsuleRadius = m_Radius * std::max(myTrans.scale.x, myTrans.scale.z);
	for (int i = 0; i < tList.size(); i++)
	{
		Vector3 P = bottomOnBoxLocal + tList[i] * axis; // 線分上のある点P
		Vector3 Q = P; // box上の最近接点Q
		Q.x = Vector3::Clamp(P.x, -extent.x, extent.x);
		Q.y = Vector3::Clamp(P.y, -extent.y, extent.y);
		Q.z = Vector3::Clamp(P.z, -extent.z, extent.z);

		// 最短距離を探す
		if (minDist > (P - Q).length())
		{
			minDist = (P - Q).length();
			pointOnBox = Q;
			pointOnCapsule = P;
		}
	}

	// ローカルからワールド
	auto LocalToWorld = [&](const Vector3& local) -> Vector3
		{
			return centerBox 
				 + boxRight   * local.x 
				 + boxUp      * local.y 
				 + boxForward * local.z;
		};

	Vector3 P_world = LocalToWorld(pointOnCapsule); // Capsule
	Vector3 Q_world = LocalToWorld(pointOnBox);		// Box
	Vector3 vect_world = P_world - Q_world; // Box → Capsule
	float dist = vect_world.length();

	// 距離の比較
	float penetration = capsuleRadius - dist;
	if (penetration <= -slop)
	{
		out.touching = false;
		out.count = 0;
		return false;
	}

	if (dist > 1e-8f)
	{
		out.normal = vect_world / dist;
		out.points[0].penetration = penetration;
	}
	else // dist がほぼ０のとき
	{
		Vector3 fallback = centerCap - centerBox;
		if (fallback.lengthSq() < 1e-8f) fallback = { 1, 0, 0 };
		out.normal = fallback.normalized(); // 退避向き
		out.points[0].penetration = penetration;
	}
	out.touching = true;

	out.points[0].pointOnA = Q_world; // Box
	out.points[0].pointOnB = P_world - out.normal * capsuleRadius; // Capsule （軸から半径分寄せた座標）
	out.count = 1;

	return out.touching;
}
// --------------------------------------------------
// CAPSULE × SPHERE（線分と球体の最近接点）
// --------------------------------------------------
bool CapsuleCollision::isOverlapWithSphere(const ColliderPose& myTrans, const SphereCollision& sphere, const ColliderPose& transSph, ContactManifold& out, float slop) const
{
	// 中心座標
	Vector3 centerCapsule = myTrans.position;
	Vector3 centerSphere  = transSph.position;

	// カプセルの円柱部の両端座標
	float half = m_CylinderHeight * myTrans.scale.y / 2.0f;
	Vector3 top = centerCapsule + half * myTrans.WorldUp();
	Vector3 bottom = centerCapsule - half * myTrans.WorldUp();
	// 軸方向算出
	Vector3 axis = top - bottom;
	if (axis.lengthSq() < 1e-8f) // 球体扱い
	{
		// 球体とみなして isOverlapWithSphere を呼ぶ
		SphereCollision capsule(m_Radius);
		return sphere.isOverlapWithSphere(transSph, capsule, myTrans, out, slop);
	}

	// 最近接点算出
	Vector3 vectBottomToSphere = centerSphere - bottom;
	float t = Vector3::Dot(vectBottomToSphere, axis) / Vector3::Dot(axis, axis);
	t = Vector3::Clamp(t, 0, 1);

	// 接触点
	Vector3 point = bottom + t * axis;

	// 距離
	Vector3 vect = centerSphere - point; // Capsule → Sphere
	float distSq = vect.lengthSq();

	// 半径
	float radiusCapsule = m_Radius * std::max(myTrans.scale.x, myTrans.scale.z);
	float radiusSphere  = sphere.Radius() * transSph.scale.x;
	float sum = radiusCapsule + radiusSphere;

	// 衝突判定
	if (distSq > 1e-12f)
	{
		const float d = sqrt(distSq);
		out.normal = -vect / d;
		out.points[0].penetration = sum - d;
	}
	else
	{
		out.normal = { 1, 0, 0 }; // 退避向き
		out.points[0].penetration = sum;
	}
	out.touching = (out.points[0].penetration >= -slop);

	out.points[0].pointOnA = point + out.normal * radiusCapsule; // Capsule
	out.points[0].pointOnB = centerSphere - out.normal * radiusSphere; // Sphere
	out.count = 1;

	return out.touching;
}
// --------------------------------------------------
// CAPSULE × CAPSULE（線分と線分の最近接点）
// --------------------------------------------------
bool CapsuleCollision::isOverlapWithCapsule(const ColliderPose& myTrans, const CapsuleCollision& capsule, const ColliderPose& transCap, ContactManifold& out, float slop) const
{
	Vector3 centerA = transCap.position;
	Vector3 centerB = myTrans.position;

	float radiusA = capsule.Radius() * std::max(transCap.scale.x, transCap.scale.z);
	float radiusB = m_Radius		 * std::max(myTrans.scale.x, myTrans.scale.z);

	// カプセルの円柱部の両端座標
	float halfA		= capsule.CylinderHeight() * transCap.scale.y / 2.0f;
	Vector3 topA	= centerA + halfA * transCap.WorldUp();
	Vector3 bottomA = centerA - halfA * transCap.WorldUp();
	float halfB		= m_CylinderHeight * myTrans.scale.y / 2.0f;
	Vector3 topB	= centerB + halfB * myTrans.WorldUp();
	Vector3 bottomB = centerB - halfB * myTrans.WorldUp();
	// 軸方向算出
	Vector3 axisA = topA - bottomA;
	Vector3 axisB = topB - bottomB;

	// 高さ０の時の処理
	if (axisA.lengthSq() < 1e-8f && axisB.lengthSq() < 1e-8f) // どっちも球体扱い
	{
		SphereCollision capsuleA(capsule.Radius());
		SphereCollision capsuleB(m_Radius);
		return capsuleA.isOverlapWithSphere(transCap, capsuleB, myTrans, out, slop);
	}
	else if (axisA.lengthSq() < 1e-8f)
	{
		SphereCollision capsuleA(capsule.Radius());
		return this->isOverlapWithSphere(myTrans, capsuleA, transCap, out, slop);
	}
	else if (axisB.lengthSq() < 1e-8f)
	{
		SphereCollision capsuleB(m_Radius);
		return capsule.isOverlapWithSphere(transCap, capsuleB, myTrans, out, slop);
	}

	// ----- 考え方 -----
	/*
		P(s) = bottomA + s * axisA
		Q(t) = bottomB + t * axisB
		D(s, t) = |P(s) - Q(t)|が最小になるような s, t を求める
		(D(s, t))^2 = |P(s) - Q(t)|^2 = | bottomA - bottomB + s*axisA - t*axisB|^2
		(D(s, t))^2 = | AB + s*axisA - t*axisB|^2 をｓとｔで偏微分すして０になるｓとｔが最小のｓとｔになる
		sで偏微分：2axisA・(AB + s*axisA - t*axisB) = 0 ⇒ axisA・(AB + s*axisA - taxisB) = 0 ⇒ s*(axisA・axisA) - t*(axisB・axisA) + AB・axisA = 0
		tで偏微分：2axisB・(AB + s*axisA - t*axisB) = 0 ⇒ axisB・(AB + s*axisA - taxisB) = 0 ⇒ s*(axisA・axisB) - t*(axisB・axisB) + AB・axisB = 0
		上記の２式からｓ、ｔを求める（以下変数に置き換える）
		as - bt + d = 0 ⇒行列で書くと [a - b ][s] = [-d]
		bs - ct + e = 0				   [b - c ][t] = [-e]
		上記の行列の行列式：Δ = ac - b^2
		Δ≠0 なら２直線は平行ではないので普通に解ける。
	*/

	// 必要な内積を算出
	// bottomA - bottomB
	Vector3 AB = bottomA - bottomB;
	float a = Vector3::Dot(axisA, axisA); // axisA の長さの２乗
	float b = Vector3::Dot(axisA, axisB); // axisA と axisB の内積
	float c = Vector3::Dot(axisB, axisB); // axisB の長さの２乗
	float d = Vector3::Dot(axisA, AB);	  // axisA と AB の内積
	float e = Vector3::Dot(axisB, AB);	  // axisB と AB の内積

	float delta = a * c - b * b;
	if (fabsf(delta) > 1e-8f) // 非平行なので連立方程式を解く
	{
		float s = (b * e - c * d) / delta;
		float t = (a * e - b * d) / delta;

		if (s > 0 && s < 1 && t > 0 && t < 1) // 0<s<1 && 0<t<1 を満たすので、s, t をそのまま使う
		{
			Vector3 pointOnAxisA = bottomA + axisA * s;
			Vector3 pointOnAxisB = bottomB + axisB * t;
			Vector3 vect = pointOnAxisB - pointOnAxisA; // A → B
			float distSq = vect.lengthSq();

			if (distSq > 1e-8f) 
			{
				float dist = vect.length();
				out.normal = vect.normalized();
				out.points[0].penetration = radiusA + radiusB - dist;

			}
			else
			{
				out.normal = { 1, 0, 0 }; // 退避向き
				out.points[0].penetration = radiusA + radiusB;
			}
			out.touching = (out.points[0].penetration >= -slop);
			out.points[0].pointOnA = pointOnAxisA + out.normal * radiusA;
			out.points[0].pointOnB = pointOnAxisB - out.normal * radiusB;

			return out.touching;
		}
	}
	// s,tが範囲外または平行なときの処理
	// s = 0, 1 または t = 0, 1 のとき
	float tList[4]{};
	float sList[4]{};
	tList[0] = 0;
	tList[1] = 1;
	tList[2] =  Vector3::Dot(axisB, AB) / c;		 // s = 0
	tList[3] =  Vector3::Dot(axisB, AB + axisA) / c; // s = 1
	sList[0] = -Vector3::Dot(axisA, AB) / a;		 // t = 0
	sList[1] = -Vector3::Dot(axisA, AB - axisB) / a; // t = 1
	sList[2] = 0;
	sList[3] = 1;
	// クランプ処理
	tList[2] = Vector3::Clamp(tList[2], 0, 1);
	tList[3] = Vector3::Clamp(tList[3], 0, 1);
	sList[0] = Vector3::Clamp(sList[0], 0, 1);
	sList[1] = Vector3::Clamp(sList[1], 0, 1);

	// 最近接点を探す
	float minDist = FLT_MAX;
	Vector3 pointOnAxisA{};
	Vector3 pointOnAxisB{};
	for (int i = 0; i < 4; i++) // 端点 vs 中点
	{
		Vector3 P = bottomA + axisA * sList[i];
		Vector3 Q = bottomB + axisB * tList[i];

		// 最短距離を探す
		if (minDist > (P - Q).length())
		{
			minDist = (P - Q).length();
			pointOnAxisA = P;
			pointOnAxisB = Q;
		}
	}
	for (int i = 0; i < 4; i++) // 端点 vs 端点
	{
		int index = i % 2;
		Vector3 P = bottomA + axisA * sList[2 + (i / 2)];// 2, 2, 3, 3
		Vector3 Q = bottomB + axisB * tList[index]; // 0, 1, 0,1 

		// 最短距離を探す
		if (minDist > (P - Q).length())
		{
			minDist = (P - Q).length();
			pointOnAxisA = P;
			pointOnAxisB = Q;
		}
	}

	Vector3 vect = pointOnAxisB - pointOnAxisA; // A → B
	float distSq = vect.lengthSq();

	if (fabsf(distSq) > 1e-8f)
	{
		float dist = vect.length();
		out.normal = vect.normalized();
		out.points[0].penetration = radiusA + radiusB - dist;
	}
	else
	{
		out.normal = { 1, 0, 0 }; // 退避向き
		out.points[0].penetration = radiusA + radiusB;
	}
	out.touching = (out.points[0].penetration >= -slop);

	out.points[0].pointOnA = pointOnAxisA + out.normal * radiusA;
	out.points[0].pointOnB = pointOnAxisB - out.normal * radiusB;
	out.count = 1;

	return out.touching;
}
// --------------------------------------------------
// CAPSULE × HEIGHTMAP
// --------------------------------------------------
bool CapsuleCollision::isOverlapWithHeightMap(const ColliderPose& myTrans, const HeightMapCollision& map, const ColliderPose& transMap, ContactManifold& out, float slop) const
{
	bool b = map.isOverlapWithCapsule(transMap, *this, myTrans, out, slop);
	out.normal = -out.normal;
	return b;
}

// ==================================================
// HeightMapCollision
// ==================================================
bool HeightMapCollision::BuildCellTrianglesWorld(int i, int j, const ColliderPose& transHM, Triangle& out0, Triangle& out1) const
{
	// 範囲チェック
	if (i < 0 || j < 0 || i >= m_Width - 1 || j >= m_Depth - 1) return false;

	// ローカル空間で４頂点を作る
	float x0 = i	   * m_CellSizeX; 
	float x1 = (i + 1) * m_CellSizeX; 
	float z0 = j	   * m_CellSizeZ; 
	float z1 = (j + 1) * m_CellSizeZ;

	float h00 = GetHeight(i	   , j    );
	float h10 = GetHeight(i + 1, j	  );
	float h01 = GetHeight(i	   , j + 1);
	float h11 = GetHeight(i + 1, j + 1);

	// ローカル頂点
	Vector3 v00(x0, h00, z0); // 左手前
	Vector3 v10(x1, h10, z0); // 右手前
	Vector3 v01(x0, h01, z1); // 左奥
	Vector3 v11(x1, h11, z1); // 右奥

	// ローカル → ワールド変換
	const Vector3 pos	  = transHM.position;
	const Vector3 right   = transHM.WorldRight();
	const Vector3 up	  = transHM.WorldUp();
	const Vector3 forward = transHM.WorldForward();

	// スケールを適用
	Vector3 scale = transHM.scale;
	v00 *= scale;
	v10 *= scale;
	v01 *= scale;
	v11 *= scale;

	auto LocalToWorld = [&](const Vector3& v)
		{
			return pos + right * v.x + up * v.y + forward * v.z;
		};

	Vector3 v00World = LocalToWorld(v00);
	Vector3 v10World = LocalToWorld(v10);
	Vector3 v01World = LocalToWorld(v01);
	Vector3 v11World = LocalToWorld(v11);

	// ２枚の三角形に分割（Meshと合わせる必要あり）
	// (v00, v01, v11), (v00, v11, v10) とする
	out0.a = v00World;
	out0.b = v01World;
	out0.c = v11World;
	out1.a = v00World;
	out1.b = v11World;
	out1.c = v10World;

	return true;
}
AABB HeightMapCollision::ComputeWorldAABB(const ColliderPose& ownerTrans) const
{
	// ※※※※※ HeightMap は回転禁止 ※※※※※（負スケーリングも禁止）
	// ローカル空間
	Vector3 minLocal = { 0.0f, m_MinHeight, 0.0f };
	Vector3 maxLocal = { m_CellSizeX * (m_Width - 1), m_MaxHeight, m_CellSizeZ * (m_Depth - 1) };

	Vector3 scale = ownerTrans.scale;

	// Scale 適用
	minLocal *= scale;
	maxLocal *= scale;

	// 原点
	Vector3 pos = ownerTrans.position;
	
	return {minLocal + pos, maxLocal + pos};
}
bool HeightMapCollision::isOverlap(const ColliderPose& myTrans, const Collision& collisionB, const ColliderPose& transB, ContactManifold& out, float slop) const
{
	return collisionB.isOverlapWithHeightMap(transB, *this, myTrans, out, slop);
}
// --------------------------------------------------
// HEIGHTMAP × BOX
// --------------------------------------------------
bool HeightMapCollision::isOverlapWithBox(const ColliderPose& myTrans, const BoxCollision& box, const ColliderPose& transBox, ContactManifold& out, float slop) const
{
	// box の情報を取得
	Vector3 centerBox  = transBox.position;
	Vector3 rightBox   = transBox.WorldRight();
	Vector3 upBox	   = transBox.WorldUp();
	Vector3 forwardBox = transBox.WorldForward();
	Vector3 halfSizeBox = box.HalfSize() * transBox.scale;

	// ローカル８頂点
	Vector3 vertexLocal[8] ={
		{ -halfSizeBox.x, -halfSizeBox.y, -halfSizeBox.z }, // 000
		{ -halfSizeBox.x, -halfSizeBox.y,  halfSizeBox.z }, // 001
		{ -halfSizeBox.x,  halfSizeBox.y, -halfSizeBox.z }, // 010
		{ -halfSizeBox.x,  halfSizeBox.y,  halfSizeBox.z }, // 011
		{  halfSizeBox.x, -halfSizeBox.y, -halfSizeBox.z }, // 100
		{  halfSizeBox.x, -halfSizeBox.y,  halfSizeBox.z }, // 101
		{  halfSizeBox.x,  halfSizeBox.y, -halfSizeBox.z }, // 110
		{  halfSizeBox.x,  halfSizeBox.y,  halfSizeBox.z } // 111
	};
	// ワールドへ変換
	auto LocalToWorld = [&](const Vector3& local)
		{
			return centerBox + rightBox * local.x + upBox * local.y + forwardBox * local.z;
		};
	Vector3 vertexes[8];
	for (int i = 0; i < 8; i++) vertexes[i] = LocalToWorld(vertexLocal[i]);
	
	// Point vs HeightMap のヘルパ
	auto TestPointOnHeightMap = [&](const Vector3& pos, TriHit& outHit)
		{
			// HeightMap の頂点とセルサイズ
			Vector3 origin = myTrans.position;
			float cellSizeX = m_CellSizeX;
			float cellSizeZ = m_CellSizeZ;

			// HeightMap のローカルXZへ変換
			Vector3 local = pos - origin;
			float fx = local.x / cellSizeX;
			float fz = local.z / cellSizeZ;

			int i = static_cast<int>(std::floor(fx));
			int j = static_cast<int>(std::floor(fz));

			// 範囲外
			if (i < 0 || j < 0 || i >= m_Width - 1 || j >= m_Depth - 1) return false;

			// セル内の[0,1]比率
			float sx = fx - static_cast<float>(i);
			float sz = fz - static_cast<float>(j);

			// 対応する２枚三角形をワールドで生成
			Triangle tri0, tri1;
			if (!BuildCellTrianglesWorld(i, j, myTrans, tri0, tri1)) return false;

			// MeshField と同じ分割に合わせる
			const Triangle* tri = (sx <= sz) ? &tri0 : &tri1;

			const Vector3& a = tri->a;
			const Vector3& b = tri->b;
			const Vector3& c = tri->c;

			// 面法線を計算
			Vector3 e1 = b - a;
			Vector3 e2 = c - a;
			Vector3 n = Vector3::Cross(e1, e2);
			float len = n.length();
			if (len < 1e-8f) return false;
			n /= len; // 正規化

			//if (n.y < 0.0f) n = -n; // 必ず上方向を向くようなチューニング

			// 平面上の高さ H(x, z) を求める
			// n・(x - a) = 0 から y を解く
			// n.x * (Px - a.x) + n.y * (y - a.y) + n.z * (Pz - a.z) = 0
			// → y = a.y - (n.x * (Px - a.x) + n.z * (Pz - a.z)) / n.y
			if (fabsf(n.y) < 1e-8f) return false;

			float H = a.y - (n.x * (pos.x - a.x) + n.z * (pos.z - a.z)) / n.y;

			float penetration = H - pos.y;

			if (penetration <= slop) return false;

			// contact 情報を詰める
			outHit.hit = true;
			outHit.penetration = penetration;
			outHit.normal = -n; // Box → HeightMap にする必要があるのでこっち向き

			// pointOnA: HeightMap 側（地形表面上の点）
			// pointOnB: Box 側（頂点そのもの）
			outHit.pointOnA = pos + n * penetration;
			outHit.pointOnB = pos;
			return true;
		};
	// ８頂点の PointHit を集める
	std::vector<TriHit> hits;
	hits.reserve(8);
	for (int i = 0; i < 8; i++)
	{
		TriHit h{};
		if (TestPointOnHeightMap(vertexes[i], h))
			hits.push_back(h);
	}
	if (hits.empty()) return false;

	// ----- TriHit から ContactManifold を組み立てる -----
	// penetration の大きい順にソート
	std::vector<const TriHit*> sorted;
	sorted.reserve(hits.size());
	for (auto& h : hits) sorted.push_back(&h);

	std::sort(sorted.begin(), sorted.end(),
		[](const TriHit* a, const TriHit* b)
		{
			return a->penetration > b->penetration;
		}
	);

	const int maxCounts = out.MAX_POINTS;

	// 先頭（最も深い点）を採用
	const TriHit* first = sorted[0];
	out.normal = first->normal; // 現在は最大深度のみを normal に採用する
	out.points[0].penetration = first->penetration;
	out.points[0].pointOnA = first->pointOnA;
	out.points[0].pointOnB = first->pointOnB;
	out.count = 1;

	// 以降、十分遠いモノだけ格納
	float minSep = 0.25f * std::min(m_CellSizeX, m_CellSizeZ); // セルサイズの1/4以上はなれていればOK
	float minSepSq = minSep * minSep;
	for (int i = 1; i < static_cast<int>(sorted.size()) && out.count < maxCounts; i++)
	{
		const TriHit* h = sorted[i];

		bool close = false;

		for (int c = 0; c < out.count; c++)
		{
			// HeightMap 側の接触点同士の距離を見る
			Vector3 diff = h->pointOnA - out.points[c].pointOnA;
			if (diff.lengthSq() < minSepSq)
			{
				close = true;
				break;
			}
		}

		if (close) continue; // 近いのでスキップ

		// 十分離れているので採用
		out.points[out.count].penetration = h->penetration;
		out.points[out.count].pointOnA = h->pointOnA;
		out.points[out.count].pointOnB = h->pointOnB;
		out.count++;
	}

	out.touching = true;

	return out.touching;
}
// --------------------------------------------------
// HEIGHTMAP × SPHERE
// --------------------------------------------------
bool HeightMapCollision::isOverlapWithSphere(const ColliderPose& myTrans, const SphereCollision& sphere, const ColliderPose& transSph, ContactManifold& out, float slop) const
{
	// Sphere ワールド情報
	Vector3 centerSph = transSph.position;
	float radius = sphere.Radius() * transSph.scale.x;

	// HeightMap の原点
	Vector3 origin = myTrans.position;

	// グリッド１マスのサイズ
	float cellSizeX = m_CellSizeX;
	float cellSizeZ = m_CellSizeZ;

	// Sphere の中心を HeightMap のローカルXZ座標に変換
	Vector3 local = centerSph - origin;
	float lx = local.x;
	float lz = local.z;

	// Sphere がかかるセル範囲を計算
	int iMin = static_cast<int>(std::floor((lx - radius) / cellSizeX));
	int iMax = static_cast<int>(std::floor((lx + radius) / cellSizeX));
	int jMin = static_cast<int>(std::floor((lz - radius) / cellSizeZ));
	int jMax = static_cast<int>(std::floor((lz + radius) / cellSizeZ));
	// HeightMap の実サイズでクランプ
	iMin = std::max(iMin, 0);
	jMin = std::max(jMin, 0);
	iMax = std::min(iMax, m_Width - 2);
	jMax = std::min(jMax, m_Depth - 2);

	// 各セルの三角形と判定
	std::vector<TriHit> hits;
	hits.reserve(16); // 適当に領域確保
	for (int j = jMin; j <= jMax; j++)
	{
		for (int i = iMin; i <= iMax; i++)
		{
			// 対応する triangle を生成
			Triangle tri0, tri1;
			if (!BuildCellTrianglesWorld(i, j, myTrans, tri0, tri1)) continue;

			TriHit hit0, hit1;

			if (CollisionTriangle::IntersectSphere(centerSph, radius, tri0, hit0))
				hits.push_back(hit0);
			if (CollisionTriangle::IntersectSphere(centerSph, radius, tri1, hit1))
				hits.push_back(hit1);
		}
	}
	if (hits.empty()) return false;

	// ----- TriHit から ContactManifold を組み立てる -----
	// penetration の大きい順にソート
	std::vector<const TriHit*> sorted;
	sorted.reserve(hits.size());
	for (auto& h : hits) sorted.push_back(&h);
	
	std::sort(sorted.begin(), sorted.end(),
			  [](const TriHit* a, const TriHit* b)
			  {
				return a->penetration > b->penetration;
			  }
			 );
	
	const int maxCounts = out.MAX_POINTS;

	// 先頭（最も深い点）を採用
	const TriHit* first = sorted[0];
	out.normal = first->normal; // 現在は最大深度のみを normal に採用する
	out.points[0].penetration = first->penetration;
	out.points[0].pointOnA = first->pointOnA;
	out.points[0].pointOnB = first->pointOnB;
	out.count = 1;

	// 以降、十分遠いモノだけ格納
	float minSep = 0.25f * std::min(m_CellSizeX, m_CellSizeZ); // セルサイズの1/4以上はなれていればOK
	float minSepSq = minSep * minSep;
	for (int i = 1; i < static_cast<int>(sorted.size()) && out.count < maxCounts; i++)
	{
		const TriHit* h = sorted[i];
	
		bool close = false;
		
		for (int c = 0; c < out.count; c++)
		{
			// Sphere 上の点同士で距離を見る
			Vector3 diff = h->pointOnA - out.points[c].pointOnA;
			if (diff.lengthSq() < minSepSq)
			{
				close = true;
				break;
			}
		}

		if (close) continue; // 近いのでスキップ

		// 十分離れているので採用
		out.points[out.count].penetration = h->penetration;
		out.points[out.count].pointOnA    = h->pointOnA;
		out.points[out.count].pointOnB    = h->pointOnB;
		out.count++;
	}

	out.touching = true;

	return out.touching;
}
// --------------------------------------------------
// HEIGHTMAP × CAPSULE
// --------------------------------------------------
bool HeightMapCollision::isOverlapWithCapsule(const ColliderPose& myTrans, const CapsuleCollision& capsule, const ColliderPose& transCap, ContactManifold& out, float slop) const
{
	// カプセルの線分を作る
	Vector3 upCap = transCap.WorldUp();
	float halfCylinderHeight = capsule.CylinderHeight() * 0.5f * transCap.scale.y;
	float radius = capsule.Radius() * transCap.scale.x;
	Vector3 centerCap = transCap.position;
	Vector3 bottom	  = centerCap - upCap * halfCylinderHeight;
	Vector3 top		  = centerCap + upCap * halfCylinderHeight;

	// サンプル球の中心を計算
	const int SAMPLE_COUNT = 5; // 調整パラメータ（３～５あたりがおすすめ）
	Vector3 centers[SAMPLE_COUNT];
	for (int i = 0; i < SAMPLE_COUNT; i++)
	{
		float  t = (SAMPLE_COUNT == 1) ? 0.5f : (float)i / float(SAMPLE_COUNT - 1);
		centers[i] = bottom + (top - bottom) * t;
	}

	// ----- Capsule 全体のAABBから HeightMap 上のセル範囲を求める -----
	// HeightMap の原点
	Vector3 origin = myTrans.position;

	// グリッド１マスのサイズ
	float cellSizeX = m_CellSizeX;
	float cellSizeZ = m_CellSizeZ;

	// カプセルのXZ投影のAABBを取る
	float minX = centers[0].x;
	float maxX = centers[0].x;
	float minZ = centers[0].z;
	float maxZ = centers[0].z;
	for (int i = 1; i < SAMPLE_COUNT; i++)
	{
		minX = std::min(minX, centers[i].x);
		maxX = std::max(maxX, centers[i].x);
		minZ = std::min(minZ, centers[i].z);
		maxZ = std::max(maxZ, centers[i].z);
	}
	// 半径分拡張
	minX -= radius;
	maxX += radius;
	minZ -= radius;
	maxZ += radius;

	// ----- HeightMap のローカルXZに変換 -----
	Vector3 minLocal = { minX - origin.x, 0.0f, minZ - origin.z };
	Vector3 maxLocal = { maxX - origin.x, 0.0f, maxZ - origin.z };

	// Capsule がかかるセル範囲を計算
	int iMin = static_cast<int>(std::floor(minLocal.x / cellSizeX));
	int iMax = static_cast<int>(std::floor(maxLocal.x / cellSizeX));
	int jMin = static_cast<int>(std::floor(minLocal.z / cellSizeZ));
	int jMax = static_cast<int>(std::floor(maxLocal.z / cellSizeZ));

	// HeightMap 範囲でクランプ
	iMin = std::max(iMin, 0);
	jMin = std::max(jMin, 0);
	iMax = std::min(iMax, m_Width - 2);
	jMax = std::min(jMax, m_Depth - 2);
	// 範囲外なら当たっていない
	if (iMin > iMax || jMin > jMax) return false;

	// ----- 各セルの三角形×サンプル球で判定 -----
	std::vector<TriHit> hits;
	hits.reserve(32);
	for (int j = jMin; j <= jMax; j++)
	{
		for (int i = iMin; i <= iMax; i++)
		{
			Triangle tri0, tri1;
			if (!BuildCellTrianglesWorld(i, j, myTrans, tri0, tri1)) continue;

			for (int s = 0; s < SAMPLE_COUNT; s++)
			{
				TriHit hit0, hit1;
				if (CollisionTriangle::IntersectSphere(centers[s], radius, tri0, hit0)) hits.push_back(hit0);
				if (CollisionTriangle::IntersectSphere(centers[s], radius, tri1, hit1)) hits.push_back(hit1);
			}
		}
	}
	if (hits.empty()) return false;

	// ----- TriHit から ContactManifold を組み立てる -----
	// penetration の大きい順にソート
	std::vector<const TriHit*> sorted;
	sorted.reserve(hits.size());
	for (auto& h : hits) sorted.push_back(&h);

	std::sort(sorted.begin(), sorted.end(),
		[](const TriHit* a, const TriHit* b)
		{
			return a->penetration > b->penetration;
		}
	);

	const int maxCounts = out.MAX_POINTS;

	// 先頭（最も深い点）を採用
	const TriHit* first = sorted[0];
	out.normal = first->normal; // 現在は最大深度のみを normal に採用する
	out.points[0].penetration = first->penetration;
	out.points[0].pointOnA = first->pointOnA;
	out.points[0].pointOnB = first->pointOnB;
	out.count = 1;

	// 以降、十分遠いモノだけ格納
	float minSep = 0.25f * std::min(m_CellSizeX, m_CellSizeZ); // セルサイズの1/4以上はなれていればOK
	float minSepSq = minSep * minSep;
	for (int i = 1; i < static_cast<int>(sorted.size()) && out.count < maxCounts; i++)
	{
		const TriHit* h = sorted[i];

		bool close = false;

		for (int c = 0; c < out.count; c++)
		{
			// Sphere 上の点同士で距離を見る
			Vector3 diff = h->pointOnA - out.points[c].pointOnA;
			if (diff.lengthSq() < minSepSq)
			{
				close = true;
				break;
			}
		}

		if (close) continue; // 近いのでスキップ

		// 十分離れているので採用
		out.points[out.count].penetration = h->penetration;
		out.points[out.count].pointOnA = h->pointOnA;
		out.points[out.count].pointOnB = h->pointOnB;
		out.count++;
	}

	out.touching = true;

	return out.touching;
}
// --------------------------------------------------
// HEIGHTMAP × HEIGHTMAP
// --------------------------------------------------
bool HeightMapCollision::isOverlapWithHeightMap(const ColliderPose& myTrans, const HeightMapCollision& map, const ColliderPose& transMap, ContactManifold& out, float slop) const
{
	// こいつらはあたらない
	return false;
}

// 備忘録
/*
// --------------------------------------------------
// BOX × BOX （SAT方式）
// --------------------------------------------------
bool BoxCollision::isOverlapWithBox(const Transform& myTrans, const BoxCollision& box, const Transform& transBox, ContactManifold& out, float slop) const
{
	// 数値安定用
	constexpr float EPS_LEN2 = 1e-10f; // 軸の長さ＾２がこれ未満なら平行とみなしスキップ
	constexpr float MARGIN	 = 1e-5f;	// 比較の緩衝

	// ----- BoxA, B の軸を１度だけ取得しておく -----
	const Vector3 aX = myTrans.GetRight();
	const Vector3 aY = myTrans.GetUp();
	const Vector3 aZ = myTrans.GetForward();
	const Vector3 bX = transBox.GetRight();
	const Vector3 bY = transBox.GetUp();
	const Vector3 bZ = transBox.GetForward();

	// ----- 軸を集める -----
	Vector3 listAxis[15];
	// BoxAの３軸
	listAxis[0] = aX;
	listAxis[1] = aY;
	listAxis[2] = aZ;
	// BoxBの３軸
	listAxis[3] = bX;
	listAxis[4] = bY;
	listAxis[5] = bZ;
	// 上の軸のクロス９軸
	int index = 6; // ６から入れるので
	for (int ia = 0; ia < 3; ia++)
	{
		for (int ib = 3; ib < 6; ib++)
		{
			Vector3 cross = Vector3::Cross(listAxis[ia], listAxis[ib]);
			if (cross.lengthSq() < EPS_LEN2) continue; // 外積が０に限りなく近い　→　ほぼ平行　→　分離軸として無効
			cross.normalize(); // 正規化
			listAxis[index++] = cross; // 正規化後に格納
		}
	}

	// ----- 各軸の判定（SAT） -----
	// Box 間のベクトル
	Vector3 vect = transBox.position - myTrans.position;
	// 各 Box の半エクステント（スケール適用）
	Vector3 extentA = {
		fabsf(myTrans.scale.x) * m_HalfSize.x,
		fabsf(myTrans.scale.y) * m_HalfSize.y,
		fabsf(myTrans.scale.z) * m_HalfSize.z,
	};
	Vector3 extentB = {
		fabsf(transBox.scale.x) * box.HalfSize().x,
		fabsf(transBox.scale.y) * box.HalfSize().y,
		fabsf(transBox.scale.z) * box.HalfSize().z,
	};

	// 各軸のチェック
	for (int i = 0; i < index; i++)
	{
		// 中心間距離を軸上に投影
		float dist = fabsf(Vector3::Dot(vect, listAxis[i]));

		// 投影半径
		float rA = extentA.x * fabsf(Vector3::Dot(aX, listAxis[i]))
				 + extentA.y * fabsf(Vector3::Dot(aY, listAxis[i]))
				 + extentA.z * fabsf(Vector3::Dot(aZ, listAxis[i]));
		float rB = extentB.x * fabsf(Vector3::Dot(bX, listAxis[i]))
				 + extentB.y * fabsf(Vector3::Dot(bY, listAxis[i]))
				 + extentB.z * fabsf(Vector3::Dot(bZ, listAxis[i]));

		if (dist > rA + rB + MARGIN) return false; // 分離しているので false
	}

	return true; // 全ての判定を通ったので true
}
*/