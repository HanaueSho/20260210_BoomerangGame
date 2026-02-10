/*
	DebugRendereComponent.h
	20251130  hanaue sho
	コライダーのワイヤーフレーム表示など
*/
#ifndef DEBUGRENDERER_H_
#define DEBUGRENDERER_H_
#include <vector>
#include <d3d11.h>
#include <wrl/client.h>
#include "Vector3.h"
#include "Vector4.h"

class Matrix4x4;

class DebugRenderer 
{
private:
	static const int MaxVertexes = 65536;
	static const int MaxIndexes = 65536;

	struct DebugVertex
	{
		Vector3 pos;
		Vector4 color;
	};

	std::vector<DebugVertex> m_Vertexes;
	std::vector<int>		 m_Indexes;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_PS;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_IL;
	
public:
	void Initialize();

	void Begin();
	void Flush();

	void DrawLine(const Vector3& p0, const Vector3& p1, const Vector4& color);
	void DrawBox(const Vector3& center, const Vector3& halfSize, const Matrix4x4& rotation, const Vector4& color);
	void DrawSphere(const Vector3& center, float radius, const Vector4& color);
	void DrawSphere(const Vector3& center, float radius, const Matrix4x4& rotation, const Vector4& color);
	void DrawCapsule(const Vector3& center, float radius, float cylinderHeight, const Matrix4x4& rotation, const Vector4& color);
};


#endif