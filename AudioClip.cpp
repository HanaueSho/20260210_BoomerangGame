/*
	AudioClip.cpp
	20251001  hanaue sho
	音源データ
*/
#include "AudioClip.h"
#include <fstream>
#include <cstring>
#include <cassert>

// --------------------------------------------------
// 本体
// --------------------------------------------------
std::unordered_map<std::string, std::weak_ptr<AudioClip>> AudioClip::s_Pool;

static inline uint32_t ReadU32LE(std::istream& is)
{
	uint8_t b[4]; is.read(reinterpret_cast<char*>(b), 4);
	return uint32_t(b[0]) | (uint32_t(b[1]) << 8) | (uint32_t(b[2]) << 16) | (uint32_t(b[3]) << 24);
}
static inline uint16_t ReadU16LE(std::istream& is)
{
	uint8_t b[2]; is.read(reinterpret_cast<char*>(b), 2);
	return uint16_t(b[0]) | (uint16_t(b[1]) << 8);
}
std::string AudioClip::NormalizePathKey(const char* path)
{
	std::string s = path ? path : "";
	for (auto& ch : s) if (ch == '\\') ch = '/';
	return s;
}

std::shared_ptr<AudioClip> AudioClip::Load(const char* path)
{
	if (!path) return nullptr;
	const std::string key = NormalizePathKey(path);

	// プール再利用
	auto it = s_Pool.find(key);
	if (it != s_Pool.end())
	{
		if (auto sp = it->second.lock()) return sp;
		s_Pool.erase(it); // 期限切れの weak_ptr を削除
	}
	auto clip = LoadInternal(key);
	if (clip) s_Pool[key] = clip;
	return clip;
}

std::shared_ptr<AudioClip> AudioClip::LoadInternal(const std::string& normPath)
{
	std::ifstream f(normPath, std::ios::binary);
	if (!f) return nullptr;

	// ----- RIFF/WAVE ヘッダ -----
	char riff[4]; f.read(riff, 4); // "RIFF"
	if (f.gcount() != 4 || std::strncmp(riff, "RIFF", 4) != 0) return nullptr;
	(void)ReadU32LE(f);			   // fileSize(未使用)
	char wave[4]; f.read(wave, 4); // "WAVE"
	if (f.gcount() != 4 || std::strncmp(wave, "WAVE", 4) != 0) return nullptr;

	// ----- チャンク走査(fmt / data を探す) -----
	WAVEFORMATEX wf{}; 
	bool hasFmt = false;
	bool hasData = false;
	std::vector<uint8_t> pcm;

	while (f && (!hasFmt || !hasData))
	{
		char id[4]; if (!f.read(id, 4)) break;
		uint32_t size = ReadU32LE(f);

		if (std::strncmp(id, "fmt ", 4) == 0)
		{
			// 最低でも PCM の WAVEFORMAT は１６バイト
			if (size < 16) return nullptr;

			// フォーマットの基礎部
			wf.wFormatTag	   = ReadU16LE(f);
			wf.nChannels	   = ReadU16LE(f);
			wf.nSamplesPerSec  = ReadU32LE(f);
			wf.nAvgBytesPerSec = ReadU32LE(f);
			wf.nBlockAlign     = ReadU16LE(f);
			wf.wBitsPerSample  = ReadU16LE(f);

			// 余り（csize 分）をスキップ
			if (size > 16)
			{
				uint16_t cbSize = ReadU16LE(f);
				wf.cbSize = cbSize;
				f.seekg(cbSize, std::ios::cur);
			}
			// 対応範囲：PCM(1) と IEEE_FLOAT(3) のみ
			if (!(wf.wFormatTag == WAVE_FORMAT_PCM || wf.wFormatTag == WAVE_FORMAT_IEEE_FLOAT))
				return nullptr;

			hasFmt = true;
		}
		else if (std::strncmp(id, "data", 4) == 0)
		{
			// データ本体
			pcm.resize(size);
			if (!f.read(reinterpret_cast<char*>(pcm.data()), size)) return nullptr;
			hasData = true;
		}
		else
		{
			// 使わないチャンクは読み飛ばす(pad 対策も兼ねて)
			f.seekg(size, std::ios::cur);
		}

		// チャンクは偶数境界にパディングされることがある
		if (size & 1) f.seekg(1, std::ios::cur);
	}

	if (!(hasFmt && hasData)) return nullptr;

	// 最小バージョン：BlockAlign とデータ長
	if (wf.nBlockAlign == 0 || (pcm.size() % wf.nBlockAlign) != 0) return nullptr;

	// 完成
	auto clip = std::make_shared<AudioClip>();
	clip->m_Format = wf;
	clip->m_PCM    = std::move(pcm);
	return clip;
}