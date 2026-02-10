/*
    AudioBank.cpp
    20251002  hanaue sho
    音源をまとめて保管する役割
*/
#include "AudioBank.h"
#include <cstring> // strcmp
#include <cctype>  // tolower
#ifndef _WIN32
#include <windows.h>
#endif

std::unordered_map<std::string, std::shared_ptr<AudioClip>> AudioBank::s_Pinned;

std::string AudioBank::NormalizePathKey(const char* path) {
    std::string s = path ? path : "";
    for (auto& ch : s) if (ch == '\\') ch = '/';
    return s;
}

bool AudioBank::IsWaveExt(const std::string& path)
{
    // 末尾の '.' を探す
    int dot = -1;
    for (int i = (int)path.size() - 1; i >= 0; i--)
    {
        if (path[i] == '.') { dot = i; break; }
        if (path[i] == '/' || path[i] == '\\') break;
    }
    if (dot < 0) return false;
    std::string ext = path.substr(dot);
    for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    return (ext == ".wav" || ext == ".wave");
}

int AudioBank::PinDirectoryImpl(const std::string& dirNormalized, bool recursive)
{
#ifndef _WIN32
    (void)dirNormalized;
    (void)recursive;
    return 0;
#else
    // WinAPI 用に '\' 区切りへ
    std::string winPath = dirNormalized;
    for (auto& ch : winPath) if (ch == '/') ch = '\\';
    if (!winPath.empty() && winPath.back() != '\\') winPath.push_back('\\');

    std::string pattern = winPath + "*";

    WIN32_FIND_DATAA fda;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &fda);
    if (hFind == INVALID_HANDLE_VALUE) return 0;

    int pinned = 0;
    do
    {
        const char* nameA = fda.cFileName;
        if (std::strcmp(nameA, ".") == 0 || std::strcmp(nameA, "..") == 0) continue;

        std::string child = winPath + nameA;
        // 正規化
        for (auto& ch : child) if (ch == '\\') ch = '/';

        const DWORD attrs = fda.dwFileAttributes;
        const bool isDir = (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;

        if (isDir)
        {
            if (recursive) pinned += PinDirectoryImpl(child, true);
        }
        else
        {
            if (!IsWaveExt(child)) continue;
            if (Pin(child.c_str())) pinned++;
        }
    } while (FindNextFileA(hFind, &fda));

    FindClose(hFind);
    return pinned;
#endif
}

bool AudioBank::Pin(const char* path)
{
    if (!path) return false;
    const std::string key = NormalizePathKey(path);

    // 既にピン済みならOK
    if (s_Pinned.find(key) != s_Pinned.end()) return true;

    // 読み込み
    auto clip = AudioClip::Load(key.c_str());
    if (!clip) return false;

    s_Pinned.emplace(key, std::move(clip));
    return true;
}

std::shared_ptr<AudioClip> AudioBank::Get(const char* path)
{
    if (!path) return nullptr;
    const std::string key = NormalizePathKey(path);

    auto it = s_Pinned.find(key);
    if (it != s_Pinned.end()) return it->second;

    // 未ピンでも取得する
    return AudioClip::Load(key.c_str());
}

void AudioBank::Unpin(const char* path)
{
    if (!path) return;
    s_Pinned.erase(NormalizePathKey(path));
}

bool AudioBank::IsPinned(const char* path)
{
    if (!path) return false;
    return s_Pinned.find(NormalizePathKey(path)) != s_Pinned.end();
}

int AudioBank::PinDirectory(const char* dirPath, bool recursive)
{
#ifndef _WIN32
    (void)dirPath;
    (void)recursive;
    return 0; // Windows 以外は未対応
#else
    if (!dirPath) return 0;
    std::string dir = NormalizePathKey(dirPath);
    if (!dir.empty() && dir.back() != '/') dir.push_back('/');
    return PinDirectoryImpl(dir, recursive);
#endif
}

void AudioBank::Clear()
{
    s_Pinned.clear();
}
