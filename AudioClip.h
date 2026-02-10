/*
	AudioClip.h
	20251001  hanaue sho
	音源データ（単一）
*/
#ifndef AUDIOCLIP_H_
#define AUDIOCLIP_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <mmreg.h> // WAVEFORMATEX
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

class AudioClip
{
private:
	// ==================================================
	// 本体要素
	// ==================================================
	// --------------------------------------------------
	// 波形をどのように解釈するかというメタ情報（説明書）
	// 形式、チャンネル数、サンプルレート、量子化ビット数等
	// --------------------------------------------------
	WAVEFORMATEX		 m_Format{}; // PCM 16bit 前提で埋める
	// --------------------------------------------------
	// 波形データ本体
	// --------------------------------------------------
	std::vector<uint8_t> m_PCM;		 // 生データ（再生中は生存している必要あり）
	// --------------------------------------------------
	// 重複ロードを避けるためのキャッシュ
	// 弱キャッシュであって所有はしない
	// --------------------------------------------------
	static std::unordered_map<std::string, std::weak_ptr<AudioClip>> s_Pool;

	// ==================================================
	// ヘルパ
	// ==================================================
	// --------------------------------------------------
	// 音源読み込み（内部処理）
	// --------------------------------------------------
	static std::shared_ptr<AudioClip> LoadInternal(const std::string& normPath);
	// --------------------------------------------------
	// パスの正規化（'\\' を '/' に統一）
	// --------------------------------------------------
	static std::string NormalizePathKey(const char* path); 

public:
	// --------------------------------------------------
	// 音源読み込み
	// --------------------------------------------------
	static std::shared_ptr<AudioClip> Load(const char* path);

	const WAVEFORMATEX& GetFormat() const { return m_Format; }
	const std::vector<uint8_t>& GetPCM() const { return m_PCM; }
};

#endif