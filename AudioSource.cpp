/*
	AudioSource.cpp
	20251001  hanaue sho
	音源を再生する機能
*/
#include "AudioSource.h"


void AudioSource::Play(bool loop)
{
	if (!m_Clip) return;

	// 初回だけ SourceVoice を作成（フォーマットに紐づく）
	if (!m_Voice)
	{
		AudioSystem::GetEngine()->CreateSourceVoice(&m_Voice, &m_Clip->GetFormat());
	}

	// キャッシュした音量を反映
	m_Voice->SetVolume(m_Volume);

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = m_Clip->GetPCM().data();
	buf.AudioBytes = (UINT32)m_Clip->GetPCM().size();
	buf.Flags	   = loop ? 0 : XAUDIO2_END_OF_STREAM;
	buf.LoopCount  = loop ? XAUDIO2_LOOP_INFINITE : 0;

	m_Voice->Stop(0);
	m_Voice->FlushSourceBuffers();
	m_Voice->SubmitSourceBuffer(&buf, nullptr);
	m_Voice->Start(0);
}

void AudioSource::Stop()
{
	if (m_Voice) 
	{
		m_Voice->Stop(0);
		m_Voice->FlushSourceBuffers();
	}
}

void AudioSource::SetVolume(float v)
{
	m_Volume = v;
	if (m_Voice)
		m_Voice->SetVolume(v);
}

void AudioSource::SetFade(float targetVolume, float fadeTime)
{
	if (m_IsFade) return; // フェード処理中は新しく更新しない
	m_IsFade = true;

	m_FadeTime = fadeTime;
	m_FadeTimer = 0.0f;
	float vol = 0; m_Voice->GetVolume(&vol);
	m_VolumeValue = (targetVolume - vol) / fadeTime;
	m_Volume = targetVolume; // キャッシュ
}

void AudioSource::Update(float dt)
{
	// ----- フェード処理 -----
	if (m_IsFade)
	{
		m_FadeTimer += dt;

		float vol = 0;
		m_Voice->GetVolume(&vol);
		printf("%f\n", vol);
		vol += m_VolumeValue * dt;
		m_Voice->SetVolume(vol);

		if (m_FadeTimer > m_FadeTime)
		{
			m_Voice->SetVolume(m_Volume);
			m_IsFade = false;
		}
	}
}

void AudioSource::Uninit()
{
	if (m_Voice)
	{
		m_Voice->DestroyVoice();
		m_Voice = nullptr;
	}
}
