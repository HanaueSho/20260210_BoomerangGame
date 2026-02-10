/*
	DxDevice.spp
	20251006  hanaue sho
	Device, DeviceContext Çê∂ê¨Ç∑ÇÈÉNÉâÉX
*/
#include "DxDevice.h"
#include <cassert>

void DxDevice::Create()
{
	UINT flags = 0;
#if defined(_DEBUG)
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	static const D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0
	};

	HRESULT hr = D3D11CreateDevice(nullptr,
								   D3D_DRIVER_TYPE_HARDWARE,
								   nullptr,
								   flags,
								   levels, _countof(levels),
								   D3D11_SDK_VERSION,
								   m_Device.GetAddressOf(),
								   &m_FeatureLevel,
								   m_DeviceContext.GetAddressOf()
								   );
	assert(SUCCEEDED(hr) && "D3D11CreateDevice failed");
}
