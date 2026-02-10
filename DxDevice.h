/*
	DxDevice.h
	20251006  hanaue sho
	Device, DeviceContext を生成するクラス
*/
#ifndef DXDEVICE_H_
#define DXDEVICE_H_
#include <wrl/client.h>
#include <d3d11.h>
using Microsoft::WRL::ComPtr;

class DxDevice
{
private:
	// ==================================================
	// ----- 本体要素 -----
	// ==================================================
	ComPtr<ID3D11Device>		m_Device;
	ComPtr<ID3D11DeviceContext> m_DeviceContext;
	D3D_FEATURE_LEVEL			m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

public:
	// ==================================================
	// ----- 関数 -----
	// ==================================================
	void Create();
	ID3D11Device*		 Device()		 const { return m_Device.Get(); }
	ID3D11DeviceContext* DeviceContext() const { return m_DeviceContext.Get(); }
	D3D_FEATURE_LEVEL	 FeatureLevel()  const { return m_FeatureLevel; }
};



#endif