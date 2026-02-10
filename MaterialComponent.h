/*
	MaterialComponent.h
	20250813  hanaue sho
	”どう描くか”という情報を持つ（シェーダー、マテリアル、テクスチャ、サンプラー）
*/
#ifndef MATERIALCOMPONENT_H_
#define MATERIALCOMPONENT_H_
#include "Component.h"
#include "Renderer.h"
#include <d3d11.h>

class MaterialComponent : public Component
{
public:
	enum class BlendMode {Opaque, Alpha, Additive};
protected:
	// ==================================================
	// ----- 構成要素 -----
	// ==================================================
	// --------------------------------------------------
	// シェーダーレイアウト（デフォルト：所有する＝解放する）
	// --------------------------------------------------
	ID3D11VertexShader* m_VS = nullptr;
	ID3D11PixelShader*  m_PS = nullptr;
	ID3D11InputLayout*  m_IL = nullptr;
	bool m_OwnsVS = true;
	bool m_OwnsPS = true;
	bool m_OwnsIL = true;

	MaterialApp	m_Material{}; // Renderer.h のMATERIAL

	// --------------------------------------------------
	// テクスチャ（通常はアセット共有＝非所有）
	// --------------------------------------------------
	ID3D11ShaderResourceView* m_MainTexture = nullptr;
	ID3D11SamplerState*		  m_Sampler		= nullptr; 
	bool m_OwnsSRV = false;
	bool m_OwnsSamp = false;

	// --------------------------------------------------
	// ブレンド設定
	// --------------------------------------------------
	BlendMode m_Blend = BlendMode::Opaque;

public:
	// ==================================================
	// ----- セット -----
	// ==================================================
	void SetVSPS(ID3D11VertexShader* vs, ID3D11PixelShader* ps, ID3D11InputLayout* il, bool takeVS = true, bool takePS = true, bool takeIL = true ) noexcept
	{	
		// 既存所有物の置き換え時はリーク防止
		if (m_OwnsVS && m_VS && m_VS != vs) m_VS->Release();
		if (m_OwnsPS && m_PS && m_PS != ps) m_PS->Release();
		if (m_OwnsIL && m_IL && m_IL != il) m_IL->Release();

		m_VS = vs; m_PS = ps; m_IL = il; 
		m_OwnsVS = takeVS; m_OwnsPS = takePS; m_OwnsIL = takeIL;
	}
	void SetMaterial(const MaterialApp& m) noexcept { m_Material = m; }
	const MaterialApp& GetMaterial() const noexcept { return m_Material; }

	void SetMainTexture(ID3D11ShaderResourceView* srv, ID3D11SamplerState* sampler, bool takeSrv = false, bool takeSamp = false) noexcept
	{ 
		if (m_OwnsSRV && m_MainTexture && m_MainTexture != srv) m_MainTexture->Release();
		if (m_OwnsSamp && m_Sampler && m_Sampler != sampler)	m_Sampler->Release();

		m_MainTexture = srv; m_Sampler = sampler; 
		m_OwnsSRV = takeSrv; m_OwnsSamp = takeSamp;
	}

	// ==================================================
	// ----- 妥当性 -----
	// ==================================================
	bool IsReady() const noexcept {	return (m_VS && m_PS && m_IL); }

	// ==================================================
	// ----- バインド -----
	// ==================================================
	void Bind() const
	{
		auto* ctx = Renderer::DeviceContext();
		if (!ctx || !IsReady()) return;

		ctx->IASetInputLayout(m_IL);
		ctx->VSSetShader(m_VS, nullptr, 0);
		ctx->PSSetShader(m_PS, nullptr, 0);

		Renderer::SetMaterial(m_Material);

		// SRV
		if (m_MainTexture) ctx->PSSetShaderResources(0, 1, &m_MainTexture);

		// サンプラー設定（未設定なら規定値）
		ID3D11SamplerState* sampl = m_Sampler ? m_Sampler : Renderer::DefaultLinearWrapSampler();
		if (sampl)		   ctx->PSSetSamplers(0, 1, &sampl);
	}

	// --------------------------------------------------
	// テクスチャを明示的に外す
	// --------------------------------------------------
	void UnbindTextures() const noexcept
	{
		auto* ctx = Renderer::DeviceContext();
		if (!ctx) return;
		ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
		ctx->PSSetShaderResources(0, 1, nullSRV);
	}

	// ==================================================
	// ----- ブレンド設定 -----
	// ==================================================
	void SetBlendMode(BlendMode m) noexcept { m_Blend = m; }
	bool IsTransparent() const noexcept { return m_Blend != BlendMode::Opaque; }

	// ==================================================
	// ----- 破棄 -----
	// ==================================================
	void Uninit() override
	{
		if (m_OwnsVS && m_VS) { m_VS->Release(); m_VS = nullptr; }
		if (m_OwnsPS && m_PS) { m_PS->Release(); m_PS = nullptr; }
		if (m_OwnsIL && m_IL) { m_IL->Release(); m_IL = nullptr; }
		if (m_OwnsSRV && m_MainTexture) { m_MainTexture->Release(); m_MainTexture = nullptr; }
		if (m_OwnsSamp && m_Sampler)	{ m_Sampler->Release();     m_Sampler = nullptr; }
	}
};

#endif