/*
	Texture.h
	20250929  hanaue sho
	テクスチャロード周りのクラス
*/
#ifndef TEXTURE_H_
#define TEXTURE_H_
#include <unordered_map>
#include <string>
#include <d3d11.h>
#include "DirectXTex.h"

class Texture
{
private:
	// --------------------------------------------------
	// テクスチャプール
	// --------------------------------------------------
	static std::unordered_map<std::string, ID3D11ShaderResourceView*> m_PoolByPath; // フルパス
	static std::unordered_map<std::string, ID3D11ShaderResourceView*> m_PoolByKey;  // キー

	// --------------------------------------------------
	// キーを作るヘルパ関数
	// --------------------------------------------------
	static std::string MakeKeyFromPath(const std::string& fullPath);

public:
	// --------------------------------------------------
	// テクスチャのロード
	// ファイルのパスから画像をロード
	// その画像をプールへ格納
	// --------------------------------------------------
	static ID3D11ShaderResourceView* LoadByPath(const char* fullPath);

	// --------------------------------------------------
	// テクスチャをロード＆登録
	// キーをファイルネームにして格納
	// --------------------------------------------------
	static ID3D11ShaderResourceView* LoadAndRegisterKey(const char* fullPath);

	// --------------------------------------------------
	// ファイルネームキーから画像を取得
	// --------------------------------------------------
	static ID3D11ShaderResourceView* GetByKey(const char* keyNoExt);

	// --------------------------------------------------
	// 指定したフォルダの中の画像を読み込む
	// --------------------------------------------------
	static int PreloadDirectory(const char* dirPath, bool recursive = true);

	// --------------------------------------------------
	// 全てのテクスチャを取得
	// --------------------------------------------------
	static int GetTextureAll(std::vector<ID3D11ShaderResourceView*>& out);

	// --------------------------------------------------
	// 指定したディレクトリのテクスチャを取得
	// --------------------------------------------------
	static int GetTexturesFromDir(const char* dirPath, std::vector<ID3D11ShaderResourceView*>& out, bool recursive = false);

	// --------------------------------------------------
	// 終了処理
	// --------------------------------------------------
	static void Shutdown();

private:
	// --------------------------------------------------
	// パスの正規化
	// --------------------------------------------------
	static std::string NormalizePathKey(const char* path);

	// --------------------------------------------------
	// ディレクトリ内の走査
	// --------------------------------------------------
	static int PreloadDirectoryImpl(const std::string& dirNormalized, bool recursive);

	// --------------------------------------------------
	// 拡張子の判定
	// --------------------------------------------------
	static bool IsWicImageExt(const std::string& path);

	// --------------------------------------------------
	// 戦闘一致判定
	// s の先頭 prefix.size() 分の文字と prefix を丸ごと比較
	// s の先頭と prefix が一致しているか判定
	// --------------------------------------------------
	static bool StartsWith(const std::string& s, const std::string& prefix);

};

#endif