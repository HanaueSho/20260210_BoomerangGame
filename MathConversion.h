/*
	MathConversion.h
	20250810  hanaue sho
    Matrix4x4とXMMATRIXの変換用ヘッダー
*/
#ifndef MATHCONVERSION_H_
#define MATHCONVERSION_H_
#include <DirectXMath.h>
#include "Matrix4x4.h"

// Matrix4x4 を XMMATRIX へ変換
inline DirectX::XMMATRIX ToXMMATRIX(const Matrix4x4& mat)
{
    DirectX::XMFLOAT4X4 temp; 
    Matrix4x4 rowMajor = mat; // ここで転置処理
    static_assert(sizeof(temp) == sizeof(rowMajor), "Matrix size mismatch");
    memcpy(&temp, &rowMajor, sizeof(temp));
    return DirectX::XMLoadFloat4x4(&temp);
}




#endif