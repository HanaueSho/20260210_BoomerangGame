/*
	SwapChainManager.cpp
	20251006  hanaue sho
	SwapChain, RTV, DSV を生成するクラス
*/
#include "SwapChainManager.h"
#include <cassert>

void SwapChainManager::Create(HWND hwnd, uint32_t width, uint32_t height, ID3D11Device* dev, ID3D11DeviceContext* ctx)
{
	m_hWnd   = hwnd;
	m_Width  = width;
	m_Height = height;
	m_Device = dev;
	m_DeviceContext = ctx;

	// DXGI Factory 取得
	ComPtr<IDXGIDevice> dxgiDevice;
	HRESULT hr = m_Device.As(&dxgiDevice);
	assert(SUCCEEDED(hr));
	ComPtr<IDXGIAdapter> adapter;
	hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
	assert(SUCCEEDED(hr));
	hr = adapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(m_Factory.GetAddressOf()));
	assert(SUCCEEDED(hr));
	
	CreateSwapChain(hwnd);
	CreateBackbufferRTV();
	CreateDepthStencil(width, height);
	SetViewport(width, height);
}

void SwapChainManager::Destroy()
{
	if (m_DeviceContext) m_DeviceContext->ClearState();
	m_DSV.Reset();
	m_RTV.Reset();
	m_SwapChain.Reset();
	m_Factory.Reset();
	m_DeviceContext.Reset();
	m_Device.Reset();
}

void SwapChainManager::Resize(uint32_t width, uint32_t height)
{
	if (width == 0 || height == 0) return; // ０サイズは無視
	if (!m_SwapChain) return;
	m_Width  = width;
	m_Height = height;

	// 最初にアンバインド（OMから外す）
	if (m_DeviceContext)
	{
		ID3D11RenderTargetView* nullRTV[1] = { nullptr };
		m_DeviceContext->OMSetRenderTargets(0, nullRTV, nullptr);
	}

	// バックバッファにぶら下がるビューは先に解放
	m_DSV.Reset();
	m_RTV.Reset();

	// バッファサイズ変更
	HRESULT hr = m_SwapChain->ResizeBuffers(0, // buffer count (0 = 変更無し) 
											width, height,
											DXGI_FORMAT_R8G8B8A8_UNORM, // 既存と同じ
											0
											);
	assert(SUCCEEDED(hr));

	CreateBackbufferRTV();
	CreateDepthStencil(width, height);
	SetViewport(width, height);
}

void SwapChainManager::BindAsRenderTarget()
{
	ID3D11RenderTargetView* rtv = m_RTV.Get();
	m_DeviceContext->OMSetRenderTargets(1, &rtv, m_DSV.Get());
	SetViewport(m_Width, m_Height);
}

void SwapChainManager::Clear(const float clearColor[4])
{
	m_DeviceContext->ClearRenderTargetView(m_RTV.Get(), clearColor);
	m_DeviceContext->ClearDepthStencilView(m_DSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void SwapChainManager::Present(int vsync)
{
	m_SwapChain->Present(vsync, 0);
}

void SwapChainManager::CreateSwapChain(HWND hwnd)
{
	DXGI_SWAP_CHAIN_DESC scd = {};
	// バッファ構成
	scd.BufferCount = 1; // バックバッファの枚数（現在はダブルバッファ）
	scd.BufferDesc.Width = m_Width;
	scd.BufferDesc.Height = m_Height;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;

	// 使用目的
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// 出力先ウィンドウ
	scd.OutputWindow = hwnd;

	// サンプリング
	scd.SampleDesc.Count = 1;	// アンチエイリアスなし
	scd.SampleDesc.Quality = 0; // 品質レベル

	// マルチサンプルを使う場合は Factory で品質を問い合わせる必要がある
	// device->CheckMultisampleQualityLevels(...);

	// ウィンドウモード／フルスクリーン
	scd.Windowed = TRUE;

	// スワップエフェクト
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // DXGI 1.0 互換
	scd.Flags = 0; // 特殊フラグなし（垂直同期オプション等で使う）

	HRESULT hr = m_Factory->CreateSwapChain(m_Device.Get(), &scd, m_SwapChain.GetAddressOf());
	assert(SUCCEEDED(hr));
}

void SwapChainManager::CreateBackbufferRTV()
{
	ComPtr<ID3D11Texture2D> backBuffer;
	HRESULT hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
	assert(SUCCEEDED(hr));

	hr = m_Device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_RTV.GetAddressOf());
	assert(SUCCEEDED(hr));
}

void SwapChainManager::CreateDepthStencil(uint32_t w, uint32_t h)
{
	D3D11_TEXTURE2D_DESC td{};
	td.Width = w;
	td.Height = h;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_D16_UNORM; // 24bit 深度 + 8bit ステンシル（今はD16へ戻した）
	td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ComPtr<ID3D11Texture2D> depthTex;
	HRESULT hr = m_Device->CreateTexture2D(&td, nullptr, depthTex.GetAddressOf());
	assert(SUCCEEDED(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd{};
	dsvd.Format = td.Format;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	hr = m_Device->CreateDepthStencilView(depthTex.Get(), &dsvd, m_DSV.GetAddressOf());
	assert(SUCCEEDED(hr));
}

void SwapChainManager::SetViewport(uint32_t w, uint32_t h)
{
	D3D11_VIEWPORT vp{};
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width  = static_cast<FLOAT>(w);
	vp.Height = static_cast<FLOAT>(h);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	m_DeviceContext->RSSetViewports(1, &vp);
}