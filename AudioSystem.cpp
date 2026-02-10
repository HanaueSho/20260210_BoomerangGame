/*
	AudioSystem.h
	20251001  hanaue sho
	音を鳴らす土台部分
*/
#include "AudioSystem.h"
#include <cassert>

#pragma comment(lib, "xaudio2.lib") // Windows SDK の XAudio2 をリンク


// --------------------------------------------------
// 本体
// --------------------------------------------------
Microsoft::WRL::ComPtr<IXAudio2> AudioSystem::s_Engine; // XAudio2 本体
IXAudio2MasteringVoice*			 AudioSystem::s_Master = nullptr; // 出力ミキサ
bool							 AudioSystem::s_Initialized = false;

bool AudioSystem::Init()
{
	if (s_Initialized) return true;

	UINT32 flags = 0;
#ifdef _DEBUG
	flags |= XAUDIO2_DEBUG_ENGINE; // デバッグ時はログを詳しく
#endif

	// ----- エンジン生成 -----
	HRESULT hr = XAudio2Create(s_Engine.GetAddressOf(), flags);
	if (FAILED(hr) || !s_Engine) return false;

#ifdef _DEBUG
	// 追加：デバッグログの詳細度（任意）
	XAUDIO2_DEBUG_CONFIGURATION dbg = {};
	dbg.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
	dbg.BreakMask = 0;
	s_Engine->SetDebugConfiguration(&dbg);
#endif

	// ----- マスターボイス生成 -----
	hr = s_Engine->CreateMasteringVoice(&s_Master/*, channels, sampleRate, 0, deviceId, nullptr*/);
	if (FAILED(hr) || !s_Master)
	{
		s_Engine.Reset();
		return false;
	}

	// 必要なら s_Engine->StartEngine(); だが通常は自動起動でOK

	s_Initialized = true;
	return true;
}

void AudioSystem::Shutdown()
{
	if (!s_Initialized) return;

	if (s_Master)
	{
		s_Master->DestroyVoice(); 
		s_Master = nullptr;
	}
	s_Engine.Reset();
	s_Initialized = false;
}

IXAudio2* AudioSystem::GetEngine()
{
	return s_Engine.Get();
}

IXAudio2MasteringVoice* AudioSystem::GetMaster()
{
	return s_Master;
}

void AudioSystem::SetMasterVolume(float vol)
{
	if (!s_Master) return;
	if (vol < 0.0f) vol = 0.0f;
	if (vol > 1.0f) vol = 1.0f;
	s_Master->SetVolume(vol); // 振幅スカラー 0.0f ~ 1.0f
}

float AudioSystem::GetMasterVolume()
{
	if (!s_Master) return 0.0f;
	float v = 0.0f;
	s_Master->GetVolume(&v);
	return v;
}

void AudioSystem::Suspend()
{
	if (s_Engine) s_Engine->StopEngine();
}

void AudioSystem::Resume()
{
	if (s_Engine) s_Engine->StartEngine();
}
