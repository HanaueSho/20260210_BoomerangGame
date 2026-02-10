/*
	Vector4 のクラス
	20251006 hanaue sho
*/
#ifndef Vector4_H_
#define Vector4_H_
#include <math.h>
#include <cassert>
#include <string>
#include "MathCommon.h"

class Vector4
{
public:
	// ==================================================
	// ----- 要素本体 -----
	// v = {x, y, z, w}
	// ==================================================
	float x = 0, y = 0, z = 0, w = 0;

	// ==================================================
	// ----- コンストラクタ -----
	// ==================================================
	Vector4() {}
	Vector4(const Vector4& a) : x(a.x), y(a.y), z(a.z), w(a.w) {}
	Vector4(float nx, float ny, float nz, float nw) : x(nx), y(ny), z(nz), w(nw) {}

	// ==================================================
	// ----- オペレータ -----
	// ==================================================
	// --------------------------------------------------
	// 標準的なオブジェクトの保守
    // --------------------------------------------------
    // --------------------------------------------------
	// 代入（C言語の慣習に従い値への参照を返す）
    // --------------------------------------------------
	Vector4& operator = (const Vector4& a)
	{ x = a.x; y = a.y; z = a.z; w = a.w; return *this; }
	// --------------------------------------------------
	// 等しさチェック
    // --------------------------------------------------
	bool operator == (const Vector4& a) const
	{ return x == a.x && y == a.y && z == a.z && w == a.w; }
	bool operator != (const Vector4& a) const
	{ return x != a.x || y != a.y || z != a.z || w != a.w; }

	// --------------------------------------------------
	// ベクトル操作
	// --------------------------------------------------
	// --------------------------------------------------
	// 加算
    // --------------------------------------------------
	Vector4 operator +() const { return *this; }
	// --------------------------------------------------
	// 単項式のマイナスは、反転したベクトルを返す
    // --------------------------------------------------
	Vector4 operator -() const { return Vector4(-x, -y, -z, -w); }
	// --------------------------------------------------
	// 二項式の＋と－はベクトルを加算し、減算する
    // --------------------------------------------------
	Vector4 operator +(const Vector4& a) const 
	{ return Vector4(x + a.x, y + a.y, z + a.z, w + a.w); }
	Vector4 operator -(const Vector4& a) const
	{ return Vector4(x - a.x, y - a.y, z - a.z, w - a.w); }
	// --------------------------------------------------
	// スカラーによる乗算と除算
    // --------------------------------------------------
	Vector4 operator *(float a) const
	{ return Vector4(x * a, y * a, z * a, w * a); }
	// --------------------------------------------------
	// ベクトルの各要素の乗算
    // --------------------------------------------------
	Vector4 operator *(const Vector4& a) const
	{
		return Vector4(x * a.x, y * a.y, z * a.z, w * a.w);
	}
	Vector4 operator /(float a) const
	{
		if (a == 0.0f) return Vector4(0, 0, 0, 0);// ※ゼロ除算チェック
		float oneOverA = 1.0f / a; 
		return Vector4(x * oneOverA, y * oneOverA, z * oneOverA, w * oneOverA); 
	}
	// --------------------------------------------------
	// 要素の取り出し
    // --------------------------------------------------
	float& operator [](int i) 
	{
		assert(0 <= i && i < 4);
		switch (i)
		{
		case 0: return x;
		case 1: return y;
		case 2: return z;
		case 3: return w;
		}
	}
	const float& operator [](int i) const
	{
		assert(0 <= i && i < 4);
		switch (i)
		{
		case 0: return x;
		case 1: return y;
		case 2: return z;
		case 3: return w;
		}
	}
	// --------------------------------------------------
	// 組み合わせ代入演算子
    // --------------------------------------------------
	Vector4& operator +=(const Vector4& a)
	{ x += a.x; y += a.y; z += a.z; w += a.w; return *this; }
	Vector4& operator -=(const Vector4& a)
	{ x -= a.x; y -= a.y; z -= a.z; w -= a.w; return *this; }
	Vector4& operator *=(const Vector4& a) // 要素ごとの乗算（アダマール積）
	{ x *= a.x; y *= a.y; z *= a.z; w *= a.w; return *this; }
	Vector4& operator /=(const Vector4& a)
	{
		if (a.x == 0 || a.y == 0 || a.z == 0 || a.w == 0) return *this;
		x /= a.x; y /= a.y; z /= a.z; w /= a.w; return *this;
	}
	Vector4& operator *=(float s) 
	{ x *= s; y *= s; z *= s; w *= s; return *this;}
	Vector4& operator /=(float s) 
	{ x /= s; y /= s; z /= s; w /= s; return *this;}
	
	// ==================================================
	// ----- ユーティリティ -----
	// ==================================================
    // --------------------------------------------------
	// ベクトルを正規化する
    // --------------------------------------------------
	void normalize()
	{
		float magSq = x * x + y * y + z * z + w * w;
		if (magSq > 0.0f)
		{
			float oneOverMag = 1.0f / sqrtf(magSq);
			x *= oneOverMag;
			y *= oneOverMag;
			z *= oneOverMag;
			w *= oneOverMag;
		}
	}
	// --------------------------------------------------
	// ベクトルの正規化のコピーを返す
    // --------------------------------------------------
	Vector4 normalized() const
	{
		float magSq = x * x + y * y + z * z + w * w;
		if (magSq > 0.0f)
		{
			float oneOverMag = 1.0f / sqrtf(magSq);
			return Vector4(x * oneOverMag, y * oneOverMag, z * oneOverMag, w * oneOverMag);
		}
		return Vector4(0, 0, 0, 0);
	}
	// --------------------------------------------------
	// ベクトルの長さを返す
	// --------------------------------------------------
	float length() const
	{
		return sqrtf(x * x + y * y + z * z + w * w);
	}
	// --------------------------------------------------
	// ベクトルの長さの二乗を返す
	// --------------------------------------------------
	float lengthSq() const
	{
		return x * x + y * y + z * z + w * w;
	}
	// --------------------------------------------------
	// ベクトルを０にする
	// --------------------------------------------------
	void zero() { x = y = z = w = 0.0f; }

	// ==================================================
	// ----- 内積-----
	// ==================================================
	// --------------------------------------------------
	// ベクトルの内積
	// --------------------------------------------------
	static float Dot(const Vector4& a, const Vector4& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}

	// ==================================================
	// ----- ゴリラヘルパ -----
	// ==================================================
	// --------------------------------------------------
	// 要素を printf で出力（意外と便利）
	// --------------------------------------------------
	static void Printf(const Vector4& v, std::string s = "noname")
	{
		printf("%s : {%f, %f, %f, %f}\n", s.c_str(), v.x, v.y, v.z, v.w);
	}
};

// ==================================================
// ----- オペレータ（逆版） -----
// ==================================================
inline Vector4 operator *(float s, const Vector4& v)
{
	return Vector4(v.x * s, v.y * s, v.z * s, v.w * s);
}
// 任意：スカラーを各成分で割った特殊用途（例えば reciprocal のような使い方）
inline Vector4 operator /(float s, const Vector4& v)
{
	return Vector4(s / v.x, s / v.y, s / v.z, s / v.w); // 任意（特殊な用途向け）
}

#endif