/*
	Matrix4x4.h
	20250806  hanaue sho
	行ベクトル流儀（４行目に平行移動成分）（右手座標系）
*/
#ifndef MATRIX4X4_H_
#define MATRIX4X4_H_
#include <cstring>
#include <math.h>
#include "Vector3.h"

class Matrix4x4
{
public:
	// ==================================================
	// ----- 要素本体 -----
	// 	   {m00, m01, m02, m03} ← Right  (m00, m01, m02)
	// m = {m10, m11, m11, m13} ← Top    (m10, m11, m12)
	//     {m20, m21, m22, m23} ← Forward(m20, m21, m22)
	//     {m30, m31, m32, m33} ← Translate(m30, m31, m32)
    // ==================================================
	float m[4][4]; // 行列本体

	// ==================================================
	// ----- コンストラクタ -----
    // ==================================================
	Matrix4x4() { identity(); } // 正規化

	// ==================================================
	// ----- オペレータ -----
    // ==================================================
    // --------------------------------------------------
	// 行列同士の掛け算
	// 	   {m00, m01, m02, m03}	     {M00, M01, M02, M03}
	// m = {m10, m11, m11, m13}, M = {M10, M11, M11, M13}
	//     {m20, m21, m22, m23}		 {M20, M21, M22, M23}
	//     {m30, m31, m32, m33}		 {M30, M31, M32, M33}
	// 
	// m * M ={m00*M00 + m01*M10 + m02*M20 + m30*M30} ... m*M[0][0]
	// --------------------------------------------------
	Matrix4x4 operator *(const Matrix4x4& rhs) const
	{
		Matrix4x4 result;
		for (int row = 0; row < 4; row++)
		{
			for (int col = 0; col < 4; col++)
			{
				result.m[row][col] = 0.0f;
				for (int k = 0; k < 4; k++)
				{
					result.m[row][col] += m[row][k] * rhs.m[k][col];
				}

			}
		}
		return result;
	}
	// --------------------------------------------------
	// 行列と float の掛け算（各要素に float を掛ける）
	// 	       {m00*s, m01*s, m02*s, m03*s}
	// m * s = {m10*s, m11*s, m11*s, m13*s}
	//		   {m20*s, m21*s, m22*s, m23*s}
	//         {m30*s, m31*s, m32*s, m33*s}
    // --------------------------------------------------
	Matrix4x4 operator *(float f) const
	{
		Matrix4x4 result;
		for (int row = 0; row < 4; row++)
		{
			for (int col = 0; col < 4; col++)
			{
				result.m[row][col] = m[row][col] * f; // 各成分に float を掛ける
			}
		}
		return result;
	}

	// ==================================================
	// ----- ユーティリティ -----
    // ==================================================
	// --------------------------------------------------
	// 単位行列に初期化
	//	   {1, 0, 0, 0}
	// m = {0, 1, 0, 0}
	//     {0, 0, 1, 0}
	//     {0, 0, 0, 1}
    // --------------------------------------------------
	void identity()
	{
		memset(m, 0, sizeof(m)); // 初期化
		m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
	}
	// --------------------------------------------------
	// 左上3x3のみをゼロに
	//	   {0, 0, 0, 0}
	// m = {0, 0, 0, 0}
	//     {0, 0, 0, 0}
	//     {0, 0, 0, 1}
    // --------------------------------------------------
	void Zero3x3()
	{
		memset(m, 0, sizeof(m)); // 初期化
		m[3][3] = 1.0f; // [3][3]のみ１，それ以外は０に
	}
	// --------------------------------------------------
	// 転置処理
	// 			{m00, m10, m20, m30}
	// transM = {m01, m11, m21, m31}
	//			{m02, m12, m22, m32}
	//			{m03, m13, m23, m33}
	// --------------------------------------------------
	Matrix4x4 Transpose() const
	{
		Matrix4x4 t;
		for (int r = 0; r < 4; ++r)
			for (int c = 0; c < 4; ++c)
				t.m[r][c] = m[c][r];
		return t;
	}
	// --------------------------------------------------
	// 逆行列
	// m * invM = 1 を満たす invM
	// ※簡易実装なので注意
	// --------------------------------------------------
	Matrix4x4 Inverse() const
	{
		Matrix4x4 inv;
		inv.identity();

		// 3x3 の逆行列を計算（回転＋スケール部分）
		float det =
			m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
			m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
			m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

		if (fabs(det) < 1e-8f) {
			// 逆行列が存在しない
			return inv; // 単位行列を返すなど適宜エラー処理
		}

		// 長さで割る
		float invDet = 1.0f / det;

		inv.m[0][0] =  (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invDet;
		inv.m[0][1] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) * invDet;
		inv.m[0][2] =  (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet;

		inv.m[1][0] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * invDet;
		inv.m[1][1] =  (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet;
		inv.m[1][2] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * invDet;

		inv.m[2][0] =  (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet;
		inv.m[2][1] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * invDet;
		inv.m[2][2] =  (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet;

		// 平行移動の逆を計算
		inv.m[3][0] = -(m[3][0] * inv.m[0][0] + m[3][1] * inv.m[1][0] + m[3][2] * inv.m[2][0]);
		inv.m[3][1] = -(m[3][0] * inv.m[0][1] + m[3][1] * inv.m[1][1] + m[3][2] * inv.m[2][1]);
		inv.m[3][2] = -(m[3][0] * inv.m[0][2] + m[3][1] * inv.m[1][2] + m[3][2] * inv.m[2][2]);

		return inv;
	}
	
	// ==================================================
	// ----- 生成 -----
	// ==================================================
	// --------------------------------------------------
	// 平行移動行列
	//	   {1, 0, 0, 0}
	// m = {0, 1, 0, 0}
	//     {0, 0, 1, 0}
	//     {x, y, z, 1}
	// --------------------------------------------------
	static Matrix4x4 CreateTranslation(float x, float y, float z)
	{
		Matrix4x4 mat;
		mat.identity();
		mat.m[3][0] = x;
		mat.m[3][1] = y;
		mat.m[3][2] = z;
		return mat;
	}
	// --------------------------------------------------
	// スケール行列
	//	   {1*s, 0  , 0  , 0}
	// m = {0  , 1*s, 0  , 0}
	//     {0  , 0  , 1*s, 0}
	//     {0  , 0  , 0  , 1}
	// --------------------------------------------------
	static Matrix4x4 CreateScale(float sx, float sy, float sz)
	{
		Matrix4x4 mat;
		mat.identity();
		mat.m[0][0] = sx;
		mat.m[1][1] = sy;
		mat.m[2][2] = sz;
		return mat;
	}
	// --------------------------------------------------
	// オイラー角から回転行列 （Yaw→Pitch→Roll）（全てラジアン）（右手座標系）
	//	   {cy * cr + sy * sp * sr, cy * sr - sy * sp * cr, sy * cp}
	// m = {-cp * sr			  , cp * cr				  , sp	   }
	//     {sy * cr + cy * sp * sr,sy * sr - cy * sp * cr , cy * cp}
	// --------------------------------------------------
	static Matrix4x4 CreateRotationYawPitchRoll(float yaw, float pitch, float roll)
	{
		float cy = cos(yaw)  , sy = sin(yaw);   // Yaw   : ｙ軸
		float cp = cos(pitch), sp = sin(pitch); // Pitch : ｘ軸
		float cr = cos(roll) , sr = sin(roll);  // Roll	 : ｚ軸

		Matrix4x4 mat;
		mat.identity();

		mat.m[0][0] = cy * cr + sy * sp * sr;
		mat.m[0][1] = cy * sr - sy * sp * cr;
		mat.m[0][2] = sy * cp;
		mat.m[0][3] = 0.0f;

		mat.m[1][0] = -cp * sr;
		mat.m[1][1] = cp * cr;
		mat.m[1][2] = sp;
		mat.m[1][3] = 0.0f;

		mat.m[2][0] = sy * cr + cy * sp * sr;
		mat.m[2][1] = sy * sr - cy * sp * cr;
		mat.m[2][2] = cy * cp;
		mat.m[2][3] = 0.0f;

		mat.m[3][0] = mat.m[3][1] = mat.m[3][2] = 0.0f;
		mat.m[3][3] = 1.0f;

		return mat;
	}
	// --------------------------------------------------
	// 回転行列（軸、角度から回転行列を作る）
	// a = angle
	// c = cos(angle), s = sin(angle), t = 1-c
	//	   { t * a.x * a.x + c	    , t * a.x * a.y + s * a.z, t * a.x * a.z - s * a.y}
	// m = { t * a.y * a.x - s * a.z, t * a.y * a.y + c 	 , t * a.y * a.z + s * a.x}
	//     { t * a.z * a.x + s * a.y, t * a.z * a.y - s * a.x, t * a.z * a.z + c 	  }
	// --------------------------------------------------
	static Matrix4x4 CreateRotationAxis(const Vector3& axis, float angle)
	{
		Vector3 a = axis.normalized();
		float c = cosf(angle);
		float s = sinf(angle);
		float t = 1.0f - c;

		Matrix4x4 mat;
		mat.identity();

		mat.m[0][0] = t * a.x * a.x + c;
		mat.m[0][1] = t * a.x * a.y + s * a.z;
		mat.m[0][2] = t * a.x * a.z - s * a.y;

		mat.m[1][0] = t * a.x * a.y - s * a.z;
		mat.m[1][1] = t * a.y * a.y + c;
		mat.m[1][2] = t * a.y * a.z + s * a.x;

		mat.m[2][0] = t * a.x * a.z + s * a.y;
		mat.m[2][1] = t * a.y * a.z - s * a.x;
		mat.m[2][2] = t * a.z * a.z + c;

		return mat;
	}
	// --------------------------------------------------
	// Transform からMatrix を生成
	// 左上3*3に回転行列を格納
	// 左上3*3にスケールを適用
	// ４行目に平行移動成分を格納
	// --------------------------------------------------
	static Matrix4x4 CreateTRS(const Vector3& pos, const Matrix4x4& rot3x3, const Vector3& s)
	{
		Matrix4x4 M = rot3x3;
		// 行スケール (S * R)
		M.m[0][0] *= s.x; M.m[0][1] *= s.x; M.m[0][2] *= s.x;
		M.m[1][0] *= s.y; M.m[1][1] *= s.y; M.m[1][2] *= s.y;
		M.m[2][0] *= s.z; M.m[2][1] *= s.z; M.m[2][2] *= s.z;
		// 平行移動 (S * R * T)
		M.m[3][0] = pos.x; M.m[3][1] = pos.y; M.m[3][2] = pos.z;
		M.m[0][3] = M.m[1][3] = M.m[2][3] = 0.0f; M.m[3][3] = 1.0f;
		return M;
	}
	// --------------------------------------------------
	// Right, Up, Forward から生成（行ベクトル流儀）
	// 	   {right.x	 , right.y  , right.z  , 0} ← Right  (m00, m01, m02)
	// m = {top.x	 , top.y	, top.z	   , 0} ← Top    (m10, m11, m12)
	//     {forward.x, forward.y, forward.z, 0} ← Forward(m20, m21, m22)
	//     {0		 , 0		, 0		   , 1} 
	// --------------------------------------------------
	static Matrix4x4 CreateAxes(const Vector3& right, const Vector3& up, const Vector3& forward, const Vector3& trans = { 0, 0, 0 })
	{
		Matrix4x4 M{};

		M.m[0][0] = right.x;	M.m[0][1] = right.y;	M.m[0][2] = right.z;	M.m[0][3] = 0.0f;
		M.m[1][0] = up.x;		M.m[1][1] = up.y;		M.m[1][2] = up.z;		M.m[1][3] = 0.0f;
		M.m[2][0] = forward.x;	M.m[2][1] = forward.y;	M.m[2][2] = forward.z;	M.m[2][3] = 0.0f;
		M.m[3][0] = trans.x;	M.m[3][1] = trans.y;	M.m[3][2] = trans.z;	M.m[3][3] = 1.0f;
		return M;
	}

	// ==================================================
	// ----- 変換・抽出 -----
	// ==================================================
	// --------------------------------------------------
	// 行列からオイラー角へ変換（右手座標系）
	// Pitch : R[1][2] から一意に決まる asin(R[1][2])
	// if (!Pitch = 90)
	//	Yaw   : R[0][2] と R[2][2] から  atan2( R[0][2], R[2][2])
	//	Roll  : R[1][0] と R[1][1] から  atan2(-R[1][0], R[1][1])
	// if (Pitch = 90)
	//	Yaw   : R[2][0] と R[0][0] から  atan2( R[2][0], R[0][0])
	//	Roll  : 0 
	// --------------------------------------------------
	Vector3 ToEulerAngles() const
	{
		float r12 = m[1][2];

		float r02 = m[0][2];
		float r22 = m[2][2];

		float r10 = m[1][0];
		float r11 = m[1][1];

		float r20 = m[2][0];
		float r00 = m[0][0];

		float pitch = asinf(Vector3::Clamp(r12, -1.0f, 1.0f));
		float yaw = 0;
		float roll = 0;

		// pitch(X軸回転)
		if (fabs(cosf(pitch)) > 1e-6f) // 
		{
			yaw  = atan2f( r02, r22); // yaw
			roll = atan2f(-r10, r11); // roll
		}
		else // ジンバルロック近傍（pitch ≒ 90）
		{
			yaw = atan2f(r20, r00);
			roll = 0.0f;
		}
		return {pitch, yaw, roll}; // {X, Y, Z}
	}
	// --------------------------------------------------
	// 行列から回転情報のみを取り出す
	// 一度回転情報部分（左上３×３）を取り出し、スケールで割り正規化
	// それを改めて新しい行列に格納し直して返す
	// --------------------------------------------------
	Matrix4x4 RotationNormalized() const
	{
		// 各行成分を取り出す
		Vector3 r0(m[0][0], m[0][1], m[0][2]); 
		Vector3 r1(m[1][0], m[1][1], m[1][2]); 
		Vector3 r2(m[2][0], m[2][1], m[2][2]); 

		// 各行のスケールを取り出す
		float sx = r0.length(); if (sx < 1e-8f) { r0 = Vector3(1, 0, 0); sx = 1; } else {r0 *= (1.0f / sx); }
		float sy = r1.length(); if (sy < 1e-8f) { r1 = Vector3(0, 1, 0); sy = 1; } else {r1 *= (1.0f / sy); }
		float sz = r2.length(); if (sz < 1e-8f) { r2 = Vector3(0, 0, 1); sz = 1; } else {r2 *= (1.0f / sz); }

		// 反転検出
		if (Vector3::Dot(Vector3::Cross(r0, r1), r2) < 0.0f) r2 *= -1.0f;

		// 正規化された回転情報を返す
		Matrix4x4 R; R.identity();
		R.m[0][0] = r0.x; R.m[0][1] = r0.y; R.m[0][2] = r0.z;
		R.m[1][0] = r1.x; R.m[1][1] = r1.y; R.m[1][2] = r1.z;
		R.m[2][0] = r2.x; R.m[2][1] = r2.y; R.m[2][2] = r2.z;
		return R;
	}

	// ==================================================
	// ----- 回転適用 -----
	// ==================================================
	// --------------------------------------------------
	// ベクトルに対して回転行列を適用
	// vect : ベクトル, Mat : 回転行列
	// v' = vect * Mat （行ベクトル流儀：右から行列をかける）
	// --------------------------------------------------
	Vector3 TransformNormal(const Vector3& vect) const
	{
		// [行][列]
		return Vector3(
			vect.x * m[0][0] + vect.y * m[1][0] + vect.z * m[2][0],
			vect.x * m[0][1] + vect.y * m[1][1] + vect.z * m[2][1],
			vect.x * m[0][2] + vect.y * m[1][2] + vect.z * m[2][2]
		);
	}
	// --------------------------------------------------
	// ベクトルに対して回転＋平行移動を適用（ワールド座標変換）
	// v = {x, y, z, 1}
	// --------------------------------------------------
	Vector3 TransformPoint(const Vector3& vect) const
	{
		return Vector3(
			vect.x * m[0][0] + vect.y * m[1][0] + vect.z * m[2][0] + m[3][0],
			vect.x * m[0][1] + vect.y * m[1][1] + vect.z * m[2][1] + m[3][1],
			vect.x * m[0][2] + vect.y * m[1][2] + vect.z * m[2][2] + m[3][2]
		);
	}

	// ==================================================
	// ----- デバッグプリント -----
	// ==================================================
	static void Printf(const Matrix4x4& mat, const std::string& name = "noname")
	{
		printf("[%s]\n", name.c_str());
		printf("{%f, %f, %f, %f}\n", mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3]);
		printf("{%f, %f, %f, %f}\n", mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3]);
		printf("{%f, %f, %f, %f}\n", mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3]);
		printf("{%f, %f, %f, %f}\n", mat.m[3][0], mat.m[3][1], mat.m[3][2], mat.m[3][3]);
	}

};

#endif