/*
	Quaternion.h
	20250806  hanaue sho
	右手系右ねじ
	q = qA * qB → Bの回転のあとにAの回転を適用
*/
#ifndef QUATERNION_H_
#define QUATERNION_H_
#include <math.h>
#include "Vector3.h"
#include "Matrix4x4.h"
#include "MathCommon.h"

class Quaternion
{
public:
	// ==================================================
	// ----- 要素本体 -----
	// q = {x, y, z, w}
	// ==================================================
	float x, y, z, w; // 本体要素

	// ==================================================
	// ----- コンストラクタ -----
	// ==================================================
	Quaternion() : x(0), y(0), z(0), w(1) { }
	Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

	// ==================================================
	// ----- オペレータ -----
	// ==================================================
    // --------------------------------------------------
	// クォータニオン同士の積（ハミルトン積：回転合成）
    // --------------------------------------------------
	Quaternion operator *(const Quaternion& rhs) const
	{
		return Quaternion(
			w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
			w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
			w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
			w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z
		);
	}
	// --------------------------------------------------
	// float との積（各要素を float 倍）
    // --------------------------------------------------
	Quaternion operator *(float s) const
	{
		return { x * s, y * s, z * s, w * s };
	}

	// ==================================================
	// ----- ユーティリティ -----
	// ==================================================
    // --------------------------------------------------
	// 単位クォータニオンを返す
    // --------------------------------------------------
	static Quaternion Identity() { return Quaternion(0, 0, 0, 1); }
    // --------------------------------------------------
	// 正規化
    // --------------------------------------------------
	Quaternion normalized() const
	{
		float magSq = x * x + y * y + z * z + w * w;
		if (magSq > 0.0f)
		{
			float invMag = 1.0f / sqrtf(magSq);
			return Quaternion(x * invMag, y * invMag, z * invMag, w * invMag);
		}
		return Identity(); // 失敗時は単位クォータニオン
	}
	// --------------------------------------------------
	// 長さ（２乗）
	// --------------------------------------------------
	float LengthSq() const noexcept { return x * x + y * y + z * z + w * w; }
	// --------------------------------------------------
	// 長さ
	// --------------------------------------------------
	float Length() const noexcept { return sqrtf(x * x + y * y + z * z + w * w); }
	// --------------------------------------------------
	// 逆元
    // --------------------------------------------------
	Quaternion Inverse(float eps = 1e-12f) const noexcept
	{
		float ls = LengthSq();
		if (ls < eps) return Identity(); // ほぼゼロ長の時は安全側に転移を返す
		return Conjugate() * (1.0f / ls);
	}
	// --------------------------------------------------
	// 共役
    // --------------------------------------------------
	Quaternion Conjugate()const noexcept { return { -x, -y, -z, w }; }

	// ==================================================
	// ----- 生成 -----
	// ==================================================
    // --------------------------------------------------
	// 軸と角度からクォータニオンを生成
	// q = {axis * sin(angle/2), cos(angle/2)}
    // --------------------------------------------------
	static Quaternion FromAxisAngle(const Vector3& axis, float angle)
	{
		Vector3 normal = axis.normalized(); // 軸を正規化
		float sinHalfAngle = sinf(angle * 0.5f);
		float cosHalfAngle = cosf(angle * 0.5f);

		return Quaternion(
			normal.x * sinHalfAngle,
			normal.y * sinHalfAngle,
			normal.z * sinHalfAngle,
			cosHalfAngle
		);
	}
	// --------------------------------------------------
	// オイラー各から生成（Yaw→Pitch→Roll）（右手座標系）
	// ※ジンバルロックは回避できないので注意（Qua->Eulaer->Quaのような行き来はズレるよ）※
    // --------------------------------------------------
	static Quaternion FromEulerAngles(const Vector3& euler)
	{
		float cy = cosf(euler.y * 0.5f), sy = sinf(euler.y * 0.5f); // yaw
		float cp = cosf(euler.x * 0.5f), sp = sinf(euler.x * 0.5f); // pitch
		float cr = cosf(euler.z * 0.5f), sr = sinf(euler.z * 0.5f); // roll
		
		Quaternion q(
			 cy * sp * cr + sy * cp * sr,
		    -cy * sp * sr + sy * cp * cr,
			 cy * cp * sr - sy * sp * cr,
			 cy * cp * cr + sy * sp * sr
		);
		return q.normalized();
	}
	// --------------------------------------------------
	// from to で作られるベクトルへ向くクォータニオンを生成
	// --------------------------------------------------
	static Quaternion LookAt(const Vector3& from, const Vector3& to, const Vector3& up = Vector3(0, 1, 0))
	{
		Vector3 forward = (to - from).normalized();
		Vector3 right = Vector3::Cross(up, forward).normalized();
		Vector3 newUp = Vector3::Cross(forward, right);

		// 回転行列を作る
		Matrix4x4 rot;
		rot.identity();

		// ちゃんと行に詰める
		rot.m[0][0] = right.x;
		rot.m[0][1] = right.y;
		rot.m[0][2] = right.z;

		rot.m[1][0] = newUp.x;
		rot.m[1][1] = newUp.y;
		rot.m[1][2] = newUp.z;

		rot.m[2][0] = forward.x;
		rot.m[2][1] = forward.y;
		rot.m[2][2] = forward.z;

		// 行列からクォータニオンを作る
		return Quaternion::FromMatrix(rot);
	}
	// --------------------------------------------------
	// 回転行列からクォータニオンを生成（※正規直交行列である必要あり）
	// 20250928 これが列ベクトル流儀のせいでバグってたみたい
	// --------------------------------------------------
	static Quaternion FromMatrix(const Matrix4x4& rot)
	{
		Quaternion q;
		float trace = rot.m[0][0] + rot.m[1][1] + rot.m[2][2];

		if (trace > 0.0f)
		{
			float s = sqrtf(trace + 1.0f) * 2.0f;
			q.w = 0.25f * s;
			q.x = (rot.m[1][2] - rot.m[2][1]) / s;
			q.y = (rot.m[2][0] - rot.m[0][2]) / s;
			q.z = (rot.m[0][1] - rot.m[1][0]) / s;
		}
		else // 最大角に応じた類似式を適用
		{
			if (rot.m[0][0] > rot.m[1][1] && rot.m[0][0] > rot.m[2][2])
			{
				float s = sqrtf(1.0f + rot.m[0][0] - rot.m[1][1] - rot.m[2][2]) * 2.0f;
				q.w = (rot.m[1][2] - rot.m[2][1]) / s;
				q.x = 0.25f * s;
				q.y = (rot.m[0][1] + rot.m[1][0]) / s;
				q.z = (rot.m[0][2] + rot.m[2][0]) / s;
			}
			else if (rot.m[1][1] > rot.m[2][2])
			{
				float s = sqrtf(1.0f + rot.m[1][1] - rot.m[0][0] - rot.m[2][2]) * 2.0f;
				q.w = (rot.m[2][0] - rot.m[0][2]) / s;
				q.x = (rot.m[0][1] + rot.m[1][0]) / s;
				q.y = 0.25f * s;
				q.z = (rot.m[1][2] + rot.m[2][1]) / s;
			}
			else
			{
				float s = sqrtf(1.0f + rot.m[2][2] - rot.m[0][0] - rot.m[1][1]) * 2.0f;
				q.w = (rot.m[0][1] - rot.m[1][0]) / s;
				q.x = (rot.m[0][2] + rot.m[2][0]) / s;
				q.y = (rot.m[1][2] + rot.m[2][1]) / s;
				q.z = 0.25f * s;
			}
		}
		return q.normalized();
	}
	// --------------------------------------------------
	// from から to へ回すためのクォータニオンを生成（差分）
	// --------------------------------------------------
	static Quaternion FromToRotation(const Vector3& from, const Vector3& to)
	{
		Vector3 v0 = from.normalized();
		Vector3 v1 = to.normalized();

		float dot = Vector3::Dot(v0, v1); //内積

		if (dot > 0.9999f)// ほぼ同じ方向
		{
			return Quaternion::Identity(); // 回転する必要なし
		}
		else if (dot < -0.9999f) // ほぼ逆方向（１８０度回転）
		{
			// 任意の垂直ベクトルを回転軸に（安定性のため）
			Vector3 orth = Vector3(1.0f, 0.0f, 0.0f);
			if (fabs(v0.x) > 0.9f) // ｘと似てるならｙを試す
				orth = Vector3(0.0f, 1.0f, 0.0f);
			Vector3 axis = Vector3::Cross(v0, orth).normalized();
			return Quaternion::FromAxisAngle(axis, PI);

		}
		else
		{
			Vector3 axis = Vector3::Cross(v0, v1).normalized();
			float angle = acosf(dot);
			return Quaternion::FromAxisAngle(axis, angle);
		}
	}
	
	// ==================================================
	// ----- 回転適用 -----
	// ==================================================
	// --------------------------------------------------
	// 与えられたベクトルをクォータニオンで回す（適用させる）
	// v' = q * v * q^(-1)
	// --------------------------------------------------
	Vector3 Rotate(const Vector3& vect) const
	{
		Quaternion qv(vect.x, vect.y, vect.z, 0.0f);
		Quaternion qn = this->normalized();
		Quaternion result = qn * qv * qn.Conjugate();

		return Vector3(result.x, result.y, result.z);
	}

	// ==================================================
	// ----- 変換・抽出 -----
	// ==================================================
	// --------------------------------------------------
	// 回転行列に変換
	// ロドリゲスの回転公式から求まる
	// --------------------------------------------------
	Matrix4x4 ToMatrix() const
	{
		float xx = x * x, yy = y * y, zz = z * z;
		float xy = x * y, xz = x * z, yz = y * z;
		float wx = w * x, wy = w * y, wz = w * z;

		Matrix4x4 mat;
		mat.identity();

		mat.m[0][0] = 1.0f - 2.0f * (yy + zz);
		mat.m[0][1] = 2.0f * (xy + wz);
		mat.m[0][2] = 2.0f * (xz - wy);

		mat.m[1][0] = 2.0f * (xy - wz);
		mat.m[1][1] = 1.0f - 2.0f * (xx + zz);
		mat.m[1][2] = 2.0f * (yz + wx);

		mat.m[2][0] = 2.0f * (xz + wy);
		mat.m[2][1] = 2.0f * (yz - wx);
		mat.m[2][2] = 1.0f - 2.0f * (xx + yy);

		return mat;
	}
	// --------------------------------------------------
	// クォータニオンからオイラー角を取得（yaw→pitch→roll）
	// 回転行列を経由しているので実質回転行列からオイラー角を求める手順と同じ
	// --------------------------------------------------
	Vector3 ToEulerAngles() const
	{
		float r12 = 2.0f * (y * z + w * x);

		float r02 = 2.0f * (x * z - w * y);
		float r22 = 1.0f - 2.0f * (x * x + y * y);

		float r10 = 2.0f * (x * y - w * z);
		float r11 = 1.0f - 2.0f * (x * x + z * z);

		float r20 = 2.0f * (x * z + w * y);
		float r00 = 1.0f - 2.0f * (y * y + z * z);

		float pitch = asinf(Vector3::Clamp(r12, -1.0f, 1.0f));
		float yaw = 0;
		float roll = 0;

		// pitch(X軸回転)
		if (fabs(cosf(pitch)) > 1e-6f) // 
		{
			yaw = atan2f(r02, r22); // yaw
			roll = atan2f(-r10, r11); // roll
		}
		else // ジンバルロック近傍（pitch ≒ 90）
		{
			yaw = atan2f(r20, r00);
			roll = 0.0f;
		}
		return { pitch, yaw, roll }; // {X, Y, Z}
	}
	// --------------------------------------------------
	// 軸と角度を取り出す
	// q = ( axis * sin(θ/2), cos(θ/2) )
	// θ = arccos(q.w) * 2
	// axis = q(x, y, z) / sin(θ/2)
	// angle が０のときは軸が任意なので、とりあえず単位ベクトル（１，０，０）を返す
	// --------------------------------------------------
	void ToAxisAngle(Vector3& outAxis, float& outAngle) const
	{
		Quaternion q = this->normalized();

		float wClamped = Vector3::Clamp(q.w, -1.0f, 1.0f);

		// 角度の取得
		float angle = 2.0f * acosf(wClamped);
		outAngle = angle;

		// 軸の取得
		float sinHalfSq = 1.0f - wClamped * wClamped; // (sinθ)^2 = 1 - cosθ^2 
		if (sinHalfSq < 0.0f) sinHalfSq = 0.0f; // 誤差で微小負になるのを防ぐ
		float sinHalf = sqrtf(sinHalfSq); 
		if (sinHalf < 0.0001f) // 角度０なので軸は任意（単位ベクトルを返す）
			outAxis = Vector3(1.0f, 0.0f, 0.0f);
		else
			outAxis = Vector3(q.x, q.y, q.z) / sinHalf;
	}

	// ==================================================
	// ----- 線形補間 -----
	// ==================================================
	// --------------------------------------------------
	// クォータニオンの球面線形補間
	// Spherical Lerp
	// --------------------------------------------------
	static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t)
	{
		Quaternion a = q1.normalized();
		Quaternion b = q2.normalized();

		// 内積で回転の角度を計算
		float  dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

		// 補完経路が遠回りになるのを防ぐ
		if (dot < 0.0f)
		{
			b = Quaternion(-b.x, -b.y, -b.z, -b.w);
			dot = -dot;
		}

		const float DOT_THRESHOULD = 0.9995f;
		if (dot > DOT_THRESHOULD)
		{
			// 線形補間（差が小さいので精度的に問題ない）
			Quaternion result = Quaternion(
				a.x + t * (b.x - a.x),
				a.y + t * (b.y - a.y),
				a.z + t * (b.z - a.z),
				a.w + t * (b.w - a.w)
			);
			return result.normalized();
		}

		// 球面線形補間
		float theta = acosf(dot);		// θ = angle between a and b
		float sinTheta = sinf(theta);	// sin(θ)
		float tTheta = theta * t;		// tθ

		float scaleA = sinf(theta - tTheta) / sinTheta;
		float scaleB = sinf(tTheta) / sinTheta;

		return Quaternion(
			a.x * scaleA + b.x * scaleB,
			a.y * scaleA + b.y * scaleB,
			a.z * scaleA + b.z * scaleB,
			a.w * scaleA + b.w * scaleB
		).normalized();
	}
	// --------------------------------------------------
	// 補間（最短距離）
	// NormalizedLerpShortest
	// --------------------------------------------------
	static Quaternion NlerpShortest(const Quaternion& q1, const Quaternion& q2, float t)
	{
		Quaternion a = q1.normalized();
		Quaternion b = q2.normalized();

		// 内積で回転の角度を計算
		float  dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

		// 補完経路が遠回りになるのを防ぐ
		if (dot < 0.0f)
		{
			b = Quaternion(-b.x, -b.y, -b.z, -b.w);
		}

		return Quaternion(
			a.x * (1-t) + b.x * t,
			a.y * (1-t) + b.y * t,
			a.z * (1-t) + b.z * t,
			a.w * (1-t) + b.w * t
		).normalized();
	}

	// ==================================================
	// ----- デバッグ用 -----
	// ==================================================
	// --------------------------------------------------
	// 二つのマトリックスの類似の確認
	// --------------------------------------------------
	static float MaxAbs3x3Diff(const Matrix4x4& A, const Matrix4x4& B) {
		float m = 0.0f;
		for (int r = 0; r < 3; r++) for (int c = 0; c < 3; c++) {
			float d = fabsf(A.m[r][c] - B.m[r][c]);
			if (d > m) m = d;
		}
		return m;
	}
	// --------------------------------------------------
	// 二つのクォータニオンの類似の確認
	// --------------------------------------------------
	static float AngleBetweenQuats(const Quaternion& a, const Quaternion& b) {
		Quaternion na = a.normalized(), nb = b.normalized();
		float dot = fabsf(na.x * nb.x + na.y * nb.y + na.z * nb.z + na.w * nb.w); // ±同値を吸収
		if (dot > 1.0f) dot = 1.0f;
		return 2.0f * acosf(dot); // [rad]
	}
};

#endif