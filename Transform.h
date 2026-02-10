/*
	Transform.h
	20250521  hanaue sho
*/
#ifndef TRANSFORM_H_
#define TRANSFORM_H_
#include <math.h>
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Quaternion.h"

struct Transform
{
public:
    // ==================================================
	// ----- 要素本体 -----
    // ==================================================
	Vector3 position = {0.0f, 0.0f, 0.0f};
	Quaternion rotation = Quaternion::Identity(); // クォータニオン
	Vector3 scale = { 1.0f, 1.0f, 1.0f };

    // ==================================================
    // ----- コンストラクタ -----
    // ==================================================
	Transform() {}

    // ==================================================
	// ----- ゲッターセッター -----
    // ==================================================
	Matrix4x4 GetLocalMatrix() const // （TransformComponentで使っています）
	{
		Matrix4x4 Scale = Matrix4x4::CreateScale(scale.x, scale.y, scale.z);
		Matrix4x4 Rotation = rotation.ToMatrix(); // 回転行列
		Matrix4x4 Translation = Matrix4x4::CreateTranslation(position.x, position.y, position.z);
		return Scale * Rotation * Translation; // S→R→T の順番
	}

	Vector3 GetRight() const	{ return rotation.Rotate(Vector3(1, 0, 0)); }
	Vector3 GetUp() const		{ return rotation.Rotate(Vector3(0, 1, 0)); }
	Vector3 GetForward() const	{ return rotation.Rotate(Vector3(0, 0, 1)); }

    // ==================================================
    // ----- ユーティリティ -----
    // ==================================================
    // --------------------------------------------------
	// MatrixからTransformを生成
    // --------------------------------------------------
    static Transform FromMatrix(const Matrix4x4& mat, float eps = 1e-6f) noexcept
    {
        Transform out;

        // １）Translation（行ベクトル：４行目が平行移動成分）
        out.position = Vector3(mat.m[3][0], mat.m[3][1], mat.m[3][2]);

        // 2) Scale：3x3 の各「行」の長さ（行ベクトル流儀）
        Vector3 row0 = Vector3(mat.m[0][0], mat.m[0][1], mat.m[0][2]);
        Vector3 row1 = Vector3(mat.m[1][0], mat.m[1][1], mat.m[1][2]);
        Vector3 row2 = Vector3(mat.m[2][0], mat.m[2][1], mat.m[2][2]);

        float sx = row0.length(); if (sx < eps) { row0 = {1, 0, 0}; sx = 1.0f; }
        float sy = row1.length(); if (sy < eps) { row1 = {0, 1, 0}; sy = 1.0f; }
        float sz = row2.length(); if (sz < eps) { row2 = {0, 0, 1}; sz = 1.0f; }

        out.scale = Vector3(sx, sy, sz);

        // 3) Rotation：行正規化（行ベクトル）
        Vector3 u0 = row0 * (1.0f / sx);
        Vector3 u1 = row1 * (1.0f / sy);
        Vector3 u2 = row2 * (1.0f / sz);

        // 反転スケール（行列の反射）対策：右手直交にそろえる
        if (Vector3::Dot(Vector3::Cross(u0, u1), u2) < 0.0f) 
        {
            // 慣例として Z を反転（どれでも良いが一貫性が大事）
            u2 *= -1.0f;
            sz = -sz;
            out.scale.z = sz;
        }
        Matrix4x4 R; R.identity();
        R.m[0][0] = u0.x; R.m[0][1] = u0.y; R.m[0][2] = u0.z;
        R.m[1][0] = u1.x; R.m[1][1] = u1.y; R.m[1][2] = u1.z;
        R.m[2][0] = u2.x; R.m[2][1] = u2.y; R.m[2][2] = u2.z;
        out.rotation = Quaternion::FromMatrix(R).normalized();

        return out;
	}
};

#endif TRANSFORM_H_