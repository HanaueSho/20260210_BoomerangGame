/*
	ModelLoader.h
	20251212  hanaue sho
	assimp によるモデルローダー
*/
#ifndef MODELLOADER_H_
#define MODELLOADER_H_
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "assimp/scene.h"	    // assimp の中間表現の構造体定義
#include "assimp/postprocess.h" // ポストプロセスフラグの定義
#include "assimp/Importer.hpp"  // Impoeter専用
#include "Matrix4x4.h"
#include "Quaternion.h"

class MeshFilterComponent;
class ID3D11ShaderResourceView;
struct AnimationClip;

// ノード
struct Node
{
	std::string name;
	int parent = -1;
	Matrix4x4 bindLocal;
};

// ボーン
struct Bone
{
	int parent; // 親インデックス（なければ -1）
	std::string name;
	Matrix4x4 bindLocal;   // バインドポーズ時のローカル Transform
	Matrix4x4 invBindPose; // 逆バインドポーズ（mesh空間→ボーン空間）
	Vector3 bindT;    // 位置バインド
	Quaternion bindR; // 回転バインド
	Vector3 bindS;	  // スケールバインド
};

// スケルトン
struct Skeleton
{
	std::vector<Bone> bones; // ボーン配列
	std::vector<Node> nodes; // ノード配列

	// 変換用
	std::unordered_map < std::string, int> nodeNameToIndex; // ノードの名前からインデックスを格納
	std::unordered_map < std::string, int> boneNameToIndex; // ボーンの名前からインデックスを格納
	std::vector<int> boneToNode; // boneIndex -> nodeIndex
	std::vector<int> meshToNode; // mesh -> node

	// ルートボーンインデックス
	int rootBone = -1;
};

struct ModelResource
{
	Skeleton skeleton;
	std::vector<AnimationClip> clip;
};

class ModelLoader
{
private:
	static std::unordered_map<std::string, std::shared_ptr<ModelResource>> s_ModelPool;
public:
	static void LoadMeshFromFile(MeshFilterComponent* filter, const std::string& path, std::vector<ID3D11ShaderResourceView*>& outSrvs, bool flipV);
	static void LoadSkinnedMeshFromFile(MeshFilterComponent* filter, const std::string& path, const Skeleton& skeleton, std::vector<ID3D11ShaderResourceView*>& outSrvs, bool flipV = false);
	static AnimationClip BuildAnimationClip(const aiScene* scene, const Skeleton& skeleton, unsigned int animeIndex);
	static AnimationClip BuildAnimationClipFromFile(const std::string& path, const Skeleton& skeleton, unsigned int animeIndex);
	static Skeleton BuildSkeleton(const aiScene* scene);
	static Skeleton BuildSkeletonFromFile(const std::string& path);
};

#endif