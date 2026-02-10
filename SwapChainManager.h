/*
	SwapChainManager.h
	20251006  hanaue sho
	SwapChain, RTV, DSV を生成するクラス
*/
#ifndef SWAPCHAINMANAGER_H_
#define SWAPCHAINMANAGER_H_
#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi.h>
#include <cstdint>
#include <windows.h>
using Microsoft::WRL::ComPtr;

class SwapChainManager
{
private:
	// ==================================================
	// ----- 本体要素 -----
	// ==================================================
	ComPtr<ID3D11Device> m_Device;
	ComPtr<ID3D11DeviceContext> m_DeviceContext;
	ComPtr<IDXGISwapChain> m_SwapChain;
	ComPtr<ID3D11RenderTargetView> m_RTV;
	ComPtr<ID3D11DepthStencilView> m_DSV;

	ComPtr<IDXGIFactory> m_Factory; // DXGI 1.0 互換のため
	HWND m_hWnd{};
	uint32_t m_Width{}, m_Height{};

public:
	// ==================================================
	// ----- 関数 -----
	// ==================================================
	void Create(HWND hwnd, uint32_t width, uint32_t height, ID3D11Device* dev, ID3D11DeviceContext* ctx);
	void Destroy();
	void Resize(uint32_t width, uint32_t height);

	void BindAsRenderTarget();
	void Clear(const float clearColor[4]);
	void Present(int vsync);

	// ==================================================
	// ----- Render::Init に渡すもの -----
	// ==================================================
	IDXGISwapChain*			SwapChain() const { return m_SwapChain.Get(); }
	ID3D11RenderTargetView* RTV()		const { return m_RTV.Get(); }
	ID3D11DepthStencilView* DSV()		const { return m_DSV.Get(); }
	uint32_t Width()  const { return m_Width; }
	uint32_t Height() const { return m_Height; }

private:
	// ==================================================
	// ----- クリエイト関数 -----
	// ==================================================
	void CreateSwapChain(HWND hwnd);
	void CreateBackbufferRTV();
	void CreateDepthStencil(uint32_t w, uint32_t h);
	void SetViewport(uint32_t w, uint32_t h);
};

#endif