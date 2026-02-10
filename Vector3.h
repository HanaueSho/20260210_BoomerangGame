/*
	Vector3 のクラス
	20250513 hanaue sho
*/
#ifndef VECTOR3_H_
#define VECTOR3_H_
#include <math.h>
#include <cassert>
#include <string>
#include "MathCommon.h"

class Vector3
{
public:
	// ==================================================
	// ----- 要素本体 -----
	// v = {x, y, z}
	// ==================================================
	float x = 0, y = 0, z = 0;

	// ==================================================
	// ----- コンストラクタ -----
	// ==================================================
	Vector3() {}
	Vector3(const Vector3& a) : x(a.x), y(a.y), z(a.z) {}
	Vector3(float nx, float ny, float nz) : x(nx), y(ny), z(nz) {}

	// ==================================================
	// ----- オペレータ -----
	// ==================================================
	// --------------------------------------------------
	// 標準的なオブジェクトの保守
    // --------------------------------------------------
    // --------------------------------------------------
	// 代入（C言語の慣習に従い値への参照を返す）
    // --------------------------------------------------
	Vector3& operator = (const Vector3& a)
	{ x = a.x; y = a.y; z = a.z; return *this; }
	// --------------------------------------------------
	// 等しさチェック
    // --------------------------------------------------
	bool operator == (const Vector3& a) const
	{ return x == a.x && y == a.y && z == a.z; }
	bool operator != (const Vector3& a) const
	{ return x != a.x || y != a.y || z != a.z; }

	// --------------------------------------------------
	// ベクトル操作
	// --------------------------------------------------
	// --------------------------------------------------
	// 加算
    // --------------------------------------------------
	Vector3 operator +() const { return *this; }
	// --------------------------------------------------
	// 単項式のマイナスは、反転したベクトルを返す
    // --------------------------------------------------
	Vector3 operator -() const { return Vector3(-x, -y, -z); }
	// --------------------------------------------------
	// 二項式の＋と－はベクトルを加算し、減算する
    // --------------------------------------------------
	Vector3 operator +(const Vector3& a) const 
	{ return Vector3(x + a.x, y + a.y, z + a.z); }
	Vector3 operator -(const Vector3& a) const
	{ return Vector3(x - a.x, y - a.y, z - a.z); }
	// --------------------------------------------------
	// スカラーによる乗算と除算
    // --------------------------------------------------
	Vector3 operator *(float a) const
	{ return Vector3(x * a, y * a, z * a); }
	// --------------------------------------------------
	// ベクトルの各要素の乗算
    // --------------------------------------------------
	Vector3 operator *(const Vector3& a) const
	{
		return Vector3(x * a.x, y * a.y, z * a.z);
	}
	Vector3 operator /(float a) const
	{
		if (a == 0.0f) return Vector3(0, 0, 0);// ※ゼロ除算チェック
		float oneOverA = 1.0f / a; 
		return Vector3(x * oneOverA, y * oneOverA, z * oneOverA); 
	}
	// --------------------------------------------------
	// 要素の取り出し
    // --------------------------------------------------
	float& operator [](int i) 
	{
		assert(0 <= i && i < 3);
		switch (i)
		{
		case 0: return x;
		case 1: return y;
		case 2: return z;
		}
	}
	const float& operator [](int i) const
	{
		assert(0 <= i && i < 3);
		switch (i)
		{
		case 0: return x;
		case 1: return y;
		case 2: return z;
		}
	}
	// --------------------------------------------------
	// 組み合わせ代入演算子
    // --------------------------------------------------
	Vector3& operator +=(const Vector3& a)
	{ x += a.x; y += a.y; z += a.z; return *this; }
	Vector3& operator -=(const Vector3& a)
	{ x -= a.x; y -= a.y; z -= a.z; return *this; }
	Vector3& operator *=(const Vector3& a) // 要素ごとの乗算（アダマール積）
	{ x *= a.x; y *= a.y; z *= a.z; return *this; }
	Vector3& operator /=(const Vector3& a)
	{
		if (a.x == 0 || a.y == 0 || a.z == 0) return *this;
		x /= a.x; y /= a.y; z /= a.z; return *this; 
	}
	Vector3& operator *=(float s) 
	{ x *= s; y *= s; z *= s; return *this;}
	Vector3& operator /=(float s) 
	{ x /= s; y /= s; z /= s; return *this;}
	
	// ==================================================
	// ----- ユーティリティ -----
	// ==================================================
    // --------------------------------------------------
	// ベクトルを正規化する
    // --------------------------------------------------
	void normalize()
	{
		float magSq = x * x + y * y + z * z;
		if (magSq > 0.0f)
		{
			float oneOverMag = 1.0f / sqrtf(magSq);
			x *= oneOverMag;
			y *= oneOverMag;
			z *= oneOverMag;
		}
	}
	// --------------------------------------------------
	// ベクトルの正規化のコピーを返す
    // --------------------------------------------------
	Vector3 normalized() const
	{
		float magSq = x * x + y * y + z * z;
		if (magSq > 0.0f)
		{
			float oneOverMag = 1.0f / sqrtf(magSq);
			return Vector3(x * oneOverMag, y * oneOverMag, z * oneOverMag);
		}
		return Vector3(0, 0, 0);
	}
	// --------------------------------------------------
	// ベクトルの長さを返す
	// --------------------------------------------------
	float length() const
	{
		return sqrtf(x * x + y * y + z * z);
	}
	// --------------------------------------------------
	// ベクトルの長さの二乗を返す
	// --------------------------------------------------
	float lengthSq() const
	{
		return x * x + y * y + z * z;
	}
	// --------------------------------------------------
	// ベクトルを０にする
	// --------------------------------------------------
	void zero() { x = y = z = 0.0f; }

	// ==================================================
	// ----- 内積・外積 -----
	// ==================================================
	// --------------------------------------------------
	// ベクトルの外積（右手系）
	// --------------------------------------------------
	static Vector3 Cross(const Vector3& a, const Vector3& b)
	{
		return Vector3(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		);
	}
	// --------------------------------------------------
	// ベクトルの外積（左手座標系）
	// --------------------------------------------------
	static Vector3 CrossLH(const Vector3& a, const Vector3& b) 
	{ 
		return -Cross(a, b); 
	}
	// --------------------------------------------------
	// ベクトルの内積
	// --------------------------------------------------
	static float Dot(const Vector3& a, const Vector3& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	// ==================================================
	// ----- ゴリラヘルパ -----
	// ==================================================
	// --------------------------------------------------
	// 要素を printf で出力（意外と便利）
	// --------------------------------------------------
	static void Printf(const Vector3& v, std::string s = "noname")
	{
		printf("%s : {%f, %f, %f}\n", s.c_str(), v.x, v.y, v.z);
	}

	// --------------------------------------------------
	// クランプ
	// 与えられた値を最小値と最大値の中に収める
	// 第１引数　：　値
	// 第２引数　：　最小値
	// 第３引数　：　最大値
	// 返り値　　：　クランプした値
	// --------------------------------------------------
	static float Clamp(const float& value, const float& min, const float& max)
	{
		if (value <= min)
			return min;
		if (value >= max)
			return max;
		return value;
	}

	// --------------------------------------------------
	// Lerp
	// --------------------------------------------------
	static Vector3 Lerp(const Vector3& a, const Vector3& b, float t)
	{
		return a * (1 - t) + b * t;
	}

};


// ==================================================
// ----- オペレータ（逆版） -----
// ==================================================
inline Vector3 operator *(float s, const Vector3& v)
{
	return Vector3(v.x * s, v.y * s, v.z * s);
}
// 任意：スカラーを各成分で割った特殊用途（例えば reciprocal のような使い方）
inline Vector3 operator /(float s, const Vector3& v)
{
	return Vector3(s / v.x, s / v.y, s / v.z); // 任意（特殊な用途向け）
}


#endif