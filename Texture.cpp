#include "Texture.h"
#include "Renderer.h"
#include "Manager.h"

#include <windows.h>
//#include <d3d11.h> // これいる？
#include "DirectXTex.h" 
#if _DEBUG
#pragma comment(lib, "DirectXTex_Debug.lib")
#else
#pragma comment(lib, "DirectXTex_Release.lib")
#endif

std::unordered_map<std::string, ID3D11ShaderResourceView*> Texture::m_PoolByPath;
std::unordered_map<std::string, ID3D11ShaderResourceView*> Texture::m_PoolByKey;

std::string Texture::MakeKeyFromPath(const std::string& fullPath)
{
	// 区切り文字を統一
	std::string s = fullPath;
	for (auto& ch : s) if (ch == '\\') ch = '/';

	// 最後のスラッシュ位置を探す
	const auto slash = s.find_last_of('/');
	const size_t start = (slash == std::string::npos) ? 0 : (slash + 1);

	// 最後のドット位置を探す
	auto dot = s.find_last_of('.');
	if (dot == std::string::npos || dot < start) dot = s.size();

	// [start, dot) を返す
	return s.substr(start, dot - start);
}

ID3D11ShaderResourceView* Texture::LoadByPath(const char* fullPath)
{
	// ----- 区切り文字を統一 -----
	std::string s = fullPath;
	for (auto& ch : s) if (ch == '\\') ch = '/';

	// ----- フルパスでロード済みならプールから返す -----
	auto it = m_PoolByPath.find(s);
	if (it != m_PoolByPath.end())
	{
		return it->second; // 見つけたので返す
	}

	// ----- マルチバイト→ワイド文字へ変換 -----
	wchar_t wFileName[512];
	const size_t CAP = sizeof(wFileName) / sizeof(wFileName[0]);
	if (s.size() + 1 > CAP) return nullptr;		// オーバーフロー防止

	size_t wrote = mbstowcs(wFileName, s.c_str(), s.size() + 1); // 第３引数はCAP（上限）と同じ
	if (wrote == (size_t)-1) return nullptr;   // 失敗チェック
	if (wrote < CAP) wFileName[wrote] = L'\0'; // 念のため終端チェック

	// ----- WIC 経由でPNG/JPG/BMP を読み込み -----
	using namespace DirectX;
	TexMetadata metadata;
	ScratchImage image;
	ID3D11ShaderResourceView* pTexture;
	HRESULT hr = LoadFromWICFile(wFileName, WIC_FLAGS_NONE, &metadata, image);
	if (FAILED(hr)) return nullptr;

	// ----- SRV の生成 -----
	hr = CreateShaderResourceView(
		Renderer::Device(), 
		image.GetImages(), 
		image.GetImageCount(), 
		metadata, 
		&pTexture);
	
	// ----- SRV が生成できたことを確認 -----
	if (FAILED(hr) || !pTexture) return nullptr;

	// ----- プールに登録 -----
	m_PoolByPath[s] = pTexture;

	return pTexture;
}

ID3D11ShaderResourceView* Texture::LoadAndRegisterKey(const char* fullPath)
{
	ID3D11ShaderResourceView* srv = LoadByPath(fullPath);
	if (!srv) return nullptr;

	std::string key = MakeKeyFromPath(fullPath);

	// 衝突はいったん考えないでプールに追加
	m_PoolByKey[key] = srv;

	return srv;
}

ID3D11ShaderResourceView* Texture::GetByKey(const char* keyNoExt)
{
	auto it = m_PoolByKey.find(keyNoExt); // 大文字小文字を区別
	return (it != m_PoolByKey.end()) ? it->second : nullptr;
}

int Texture::PreloadDirectory(const char* dirPath, bool recursive)
{
	if (!dirPath) return 0;

	std::string dir = NormalizePathKey(dirPath); // 正規化してから実装関数へ
	return PreloadDirectoryImpl(dir, recursive);
}

int Texture::GetTextureAll(std::vector<ID3D11ShaderResourceView*>& out)
{
	const size_t before = out.size();
	out.reserve(out.size() + m_PoolByPath.size());
	for (const auto& kv : m_PoolByPath)
		out.push_back(kv.second);
	return static_cast<int>(out.size() - before);
}

int Texture::GetTexturesFromDir(const char* dirPath, std::vector<ID3D11ShaderResourceView*>& out, bool recursive)
{
	if (!dirPath) return 0;

	std::string dir = NormalizePathKey(dirPath); // 正規化
	if (dir.back() != '/') dir += "/"; // 末尾にスラッシュが無いなら付与

	const size_t before = out.size();

	for (const auto& kv : m_PoolByPath)
	{
		const std::string& path = kv.first;
		if (!StartsWith(path, dir)) continue;

		if (!recursive)
		{
			// 直下のみ：dir の直後に更なる'/'があればサブフォルダなので除外
			size_t pos = path.find('/', dir.size());
			if (pos != std::string::npos) continue;
		}
		out.push_back(kv.second);
	}
	return static_cast<int>(out.size() - before);
}

// 全開放
void Texture::Shutdown()
{
	for (auto& kv : m_PoolByPath)
		if (kv.second) kv.second->Release();
	m_PoolByPath.clear();
	m_PoolByKey.clear();
}

std::string Texture::NormalizePathKey(const char* path)
{
	std::string s = path ? path : "";
	for (auto& ch : s) if (ch == '\\') ch = '/';
	return s;
}

int Texture::PreloadDirectoryImpl(const std::string& dirNormalized, bool recursive)
{
	// '\\'区切りの windows パスを作成
	// winAPI で検索するために一度 '\\'区切りへ戻す
	std::string winPath = dirNormalized;
	for (auto& ch : winPath) if (ch == '/') ch = '\\';
	if (!winPath.empty() && winPath.back() != '\\') winPath.push_back('\\');

	// 検索パターン（ワイルドカード）
	std::string pattern = winPath + "*";

	// ANSI 版の検索を開始
	WIN32_FIND_DATAA fda;
	HANDLE hFind = FindFirstFileA(pattern.c_str(), &fda);
	if (hFind == INVALID_HANDLE_VALUE) return 0; // 失敗なら０を返す

	int loaded = 0;

	do 	
	{
		// 見つかったエントリ名
		const char* nameA = fda.cFileName;
		if (std::strcmp(nameA, ".") == 0 || std::strcmp(nameA, "..") == 0) continue;

		// 子のフルパス（ANSI）
		std::string child = winPath + nameA;

		// エンジン内部表現へ正規化
		for (auto& ch : child) if (ch == '\\') ch = '/';

		// ディレクトリかファイルかを属性で判定
		const bool isDir = (fda.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		if (isDir)
		{
			// ディレクトリなら、 recursive = true のときに再帰的に処理
			if (recursive) loaded += PreloadDirectoryImpl(child, true);
		}
		else
		{
			// ファイルなら拡張子が WIC 画像か判定
			if (!IsWicImageExt(child)) continue;
			// 画像ならロード＆キー登録
			if (Texture::LoadAndRegisterKey(child.c_str())) ++loaded;
		}
	} while (FindNextFileA(hFind, &fda)); // 次のエントリへ

	// ハンドルを閉じる
	FindClose(hFind);
	return loaded;
}

bool Texture::IsWicImageExt(const std::string& path)
{
	// 小文字拡張子を取得
	int dot = -1;
	for (int i = (int)path.size() - 1; i >= 0; i--)
	{
		if (path[i] == '.') { dot = i; break; }
		if (path[i] == '/' || path[i] == '\\') break; // ディレクトリを跨いだら中断
	}
	if (dot < 0) return false;

	std::string ext = path.substr(dot); // 拡張子部分を取り出す
	for (auto& c : ext) c = (char)std::tolower((unsigned char)c); // 拡張子部分を小文字化

	return ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
		   ext == ".bmp" || ext == ".gif" || ext == ".tif"  ||
		   ext == ".tiff";
}

bool Texture::StartsWith(const std::string& s, const std::string& prefix)
{
	return s.size() >= prefix.size() && std::equal(prefix.begin(), prefix.end(), s.begin());
}

