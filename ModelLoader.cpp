/*
	ModelLoader.cpp
	20251212  hanaue sho
	assimp によるモデルローダー
	※ TexCoord.y を反転させています
*/
#pragma comment (lib, "assimp-vc143-mt.lib") // ライブラリリンク
#include <cassert>
#include <filesystem>
#include "ModelLoader.h"
#include "DirectXTex.h" // LoadFromWICMemory
#include "MeshFilterComponent.h"
#include "Renderer.h"
#include "Texture.h"
#include "AnimationClip.h"

std::unordered_map<std::string, std::shared_ptr<ModelResource>> ModelLoader::s_ModelPool;

namespace
{
	// TRSに分解
	void ExtractBindTRS(const Matrix4x4& M, Vector3& outT, Quaternion& outR, Vector3& outS)
	{
		outT = Vector3(M.m[3][0], M.m[3][1], M.m[3][2]);

		Vector3 r0(M.m[0][0], M.m[0][1], M.m[0][2]);
		Vector3 r1(M.m[1][0], M.m[1][1], M.m[1][2]);
		Vector3 r2(M.m[2][0], M.m[2][1], M.m[2][2]);

		float sx = r0.length(); if (sx < 1e-8f) { sx = 1.0f; r0 = Vector3(1, 0, 0); }
		float sy = r1.length(); if (sy < 1e-8f) { sy = 1.0f; r1 = Vector3(0, 1, 0); }
		float sz = r2.length(); if (sz < 1e-8f) { sz = 1.0f; r2 = Vector3(0, 0, 1); }

		outS = Vector3(sx, sy, sz);

		r0 *= (1.0f / sx);
		r1 *= (1.0f / sy);
		r2 *= (1.0f / sz);

		if (Vector3::Dot(Vector3::Cross(r0, r1), r2) < 0.0f)
		{
			r2 *= -1.0f;
		}

		Matrix4x4 mat; 
		mat.identity();
		mat.m[0][0] = r0.x; mat.m[0][1] = r0.y; mat.m[0][2] = r0.z;
		mat.m[1][0] = r1.x; mat.m[1][1] = r1.y; mat.m[1][2] = r1.z;
		mat.m[2][0] = r2.x; mat.m[2][1] = r2.y; mat.m[2][2] = r2.z;
		outR = Quaternion::FromMatrix(mat);
	}

	// aiMatrix4x4 から Matrix4x4 へ変換
	Matrix4x4 ToMatrix4x4(const aiMatrix4x4& m)
	{
		Matrix4x4 mat;
		mat.m[0][0] = (float)m.a1; mat.m[0][1] = (float)m.b1; mat.m[0][2] = (float)m.c1; mat.m[0][3] = (float)m.d1;
		mat.m[1][0] = (float)m.a2; mat.m[1][1] = (float)m.b2; mat.m[1][2] = (float)m.c2; mat.m[1][3] = (float)m.d2;
		mat.m[2][0] = (float)m.a3; mat.m[2][1] = (float)m.b3; mat.m[2][2] = (float)m.c3; mat.m[2][3] = (float)m.d3;
		mat.m[3][0] = (float)m.a4; mat.m[3][1] = (float)m.b4; mat.m[3][2] = (float)m.c4; mat.m[3][3] = (float)m.d4;
		return mat;
	}

	// ノードを集める
	void BuildNodesRecursive(const aiNode* node, int parentNode, Skeleton& skeleton)
	{
		Node n;
		n.name = node->mName.C_Str();
		n.parent = parentNode;
		n.bindLocal = ToMatrix4x4(node->mTransformation);

		// 重複確認
		auto it = skeleton.nodeNameToIndex.find(n.name);
		if (it != skeleton.nodeNameToIndex.end())
		{
			assert(false && "error");
		}

		int thisIndex = (int)skeleton.nodes.size();
		skeleton.nodes.push_back(n);
		skeleton.nodeNameToIndex[n.name] = thisIndex;

		// ----- meshToNode 登録 -----
		for (unsigned int i = 0;i < node->mNumMeshes; i++)
		{
			unsigned int meshIndex = node->mMeshes[i];

			if (meshIndex >= skeleton.meshToNode.size())
			{
				printf("[BuildNodesRecursive] meshIndex out of range: %u\n", meshIndex);
				continue;
			}
			if (skeleton.meshToNode[meshIndex] != -1)
			{
				printf("[BuildNodesRecursive] meshIndex %u already assigned to node %d, now %d (node=%s)\n",
					meshIndex, skeleton.meshToNode[meshIndex], thisIndex, n.name.c_str());
			}
			skeleton.meshToNode[meshIndex] = thisIndex;
		}

		// 子へ
		for (unsigned int i = 0; i < node->mNumChildren; i++)
			BuildNodesRecursive(node->mChildren[i], thisIndex, skeleton);
	}

	// aiBone を集める
	// scene->mesh->bone
	std::unordered_map<std::string, const aiBone*> CollectBones(const aiScene* scene)
	{
		std::unordered_map<std::string, const aiBone*> map;

		// mesh の数分回す
		for (unsigned int m = 0; m < scene->mNumMeshes; m++)
		{
			const aiMesh* mesh = scene->mMeshes[m];
			// bone の数分回す
			for (unsigned int b = 0; b < mesh->mNumBones; b++)
			{
				const aiBone* bone = mesh->mBones[b];
				std::string name = bone->mName.C_Str();

				if (name.empty()) continue;

				// bone 格納
				// emplace(key, value) : 
				// key が存在しないなら新規に要素を作って挿入
				// key が既に存在するなら何もしない（unordered_map はキー重複を許さないので）
				// 戻り値は it は要素を指すイテレーター、 inserted はboolを返す（今回の挿入が成功したかどうか）
				auto [it, inserted] = map.emplace(name, bone);
				if (!inserted)
				{
					//it->second = bone;
					printf("重複があります: %s\n", name.c_str());
				}
			}
		}
		return map;
	}

	// 親骨探索
	int FindParentBoneIndexByNode(const Skeleton& skeleton, int nodeIndex)
	{
		int p = skeleton.nodes[nodeIndex].parent;
		while (p >= 0)
		{
			auto it = skeleton.boneNameToIndex.find(skeleton.nodes[p].name);
			if (it != skeleton.boneNameToIndex.end())
				return it->second; // 親骨が見つかった
			p = skeleton.nodes[p].parent;
		}
		return -1; // ルート骨
	}

	// 頂点にウェイトを追加する
	constexpr int MAX_INFLUENCES = 4;
	void AddBoneWeightToVertex(VERTEX_SKINNED& vertex, int boneIndex, float weight)
	{
		if (weight <= 0.0f) return;

		// まだ空いているスロットがあるならそこに入れる
		for (int i = 0; i < MAX_INFLUENCES; i++)
		{
			if (vertex.boneWeight[i] <= 0.0f)
			{
				vertex.boneIndex[i] = static_cast<uint8_t>(boneIndex);
				vertex.boneWeight[i] = weight;
				return;
			}
		}

		// 全部埋まっている場合は「一番小さいウェイトと差し替え」（重いボーン優先）
		int smallest = 0;
		for (int i = 0; i < MAX_INFLUENCES; i++)
		{
			if (vertex.boneWeight[i] < vertex.boneWeight[smallest])
				smallest = i;
		}
		if (vertex.boneWeight[smallest] < weight)
		{
			vertex.boneIndex[smallest] = static_cast<uint8_t>(boneIndex);
			vertex.boneWeight[smallest] = weight;
		}
	}

	// VERTEX_SKINNED に情報を詰める
	void FillSkinnedVertexesFromMesh(const aiMesh* mesh, const Skeleton& skeleton, std::vector<VERTEX_SKINNED>& outVertexes, bool flipV)
	{
		assert(mesh);
		outVertexes.clear();
		outVertexes.resize(mesh->mNumVertices);

		// 最初に position, normal, texcoord をセット＆ボーン情報をゼロ初期化
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			auto& vertex = outVertexes[i];

			// position
			vertex.position = Vector3(
				mesh->mVertices[i].x,
				mesh->mVertices[i].y,
				mesh->mVertices[i].z
			);
			// normal
			if (mesh->HasNormals())
			{
				vertex.normal = Vector3(
					mesh->mNormals[i].x,
					mesh->mNormals[i].y,
					mesh->mNormals[i].z
				);
			}
			else
				vertex.normal = Vector3(0, 1, 0); // 適当なデフォルト値
			// texcord
			if (mesh->HasTextureCoords(0))
			{
				float u = mesh->mTextureCoords[0][i].x;
				float v = mesh->mTextureCoords[0][i].y;
				if (flipV) v = 1.0f - v;
				vertex.texcoord = Vector2(u, v);
			}
			else
				vertex.texcoord = Vector2(0, 0);

			// ボーン情報はセゼロ期化
			for (int j = 0; j < MAX_INFLUENCES; j++)
			{
				vertex.boneIndex[j] = 0;
				vertex.boneWeight[j] = 0;
			}
		}

		// Skeleton の名前 → index のマップを作る
		const std::unordered_map<std::string, int>& boneNameToIndex = skeleton.boneNameToIndex;

		// 各 aiBone について
		for (unsigned int b = 0; b < mesh->mNumBones; b++)
		{
			const aiBone* aiBonePtr = mesh->mBones[b];
			if (!aiBonePtr) continue;

			std::string boneName = aiBonePtr->mName.C_Str();
			auto it = boneNameToIndex.find(boneName);
			if (it == boneNameToIndex.end()) // Skeleton に存在しないボーン名は無視
				continue;

			int boneIndex = it->second;

			// このボーンが影響する全頂点に対してウェイトを追加
			for (unsigned int w = 0; w < aiBonePtr->mNumWeights; w++)
			{
				const aiVertexWeight& vw = aiBonePtr->mWeights[w];

				unsigned int vId = vw.mVertexId;
				float		 weight = vw.mWeight;
				if (vId >= outVertexes.size()) continue;
				AddBoneWeightToVertex(outVertexes[vId], boneIndex, weight);
			}
		}

		// ウェイト正規化＆フォールバック
		for (auto& vertex : outVertexes)
		{
			float sum = vertex.boneWeight[0] +
				vertex.boneWeight[1] +
				vertex.boneWeight[2] +
				vertex.boneWeight[3];
			if (sum > 0.0f)
			{
				float inv = 1.0f / sum;
				for (int i = 0; i < MAX_INFLUENCES; i++)
					vertex.boneWeight[i] *= inv;
			}
			else // どのボーンにも紐づいていない頂点
			{
				vertex.boneIndex[0] = (uint8_t)skeleton.rootBone; // ルートボーンへフォールバック
				vertex.boneWeight[0] = 1.0f;
			}
		}
	}

	// ボーンのソート
	void SortBonesParentFirst(Skeleton& skeleton)
	{
		const int n = (int)skeleton.bones.size();
		if (n <= 1) return;

		// children を作る
		std::vector<std::vector<int>> children(n);
		std::vector<int> roots;
		roots.reserve(n);

		for (int i = 0; i < n; i++)
		{
			int p = skeleton.bones[i].parent;
			if (p < 0)
			{
				roots.push_back(i);
				continue;
			}

			if (p >= n) // 壊れた親参照用。ルート扱いに落とす（回避策）
			{
				skeleton.bones[i].parent = -1;
				roots.push_back(i);
				continue;
			}
			children[p].push_back(i);
		}

		// 名前で兄弟順を固定
		auto boneKey = [&](int idx) -> const std::string& {
			return skeleton.bones[idx].name;
			};

		// roots を固定順に
		std::sort(roots.begin(), roots.end(), [&](int a, int b) {
			return boneKey(a) < boneKey(b);
			});

		// 各親の children を固定順に
		for (int p = 0; p < n; ++p) {
			auto& ch = children[p];
			std::sort(ch.begin(), ch.end(), [&](int a, int b) {
				return boneKey(a) < boneKey(b);
				});
		}


		// 親→子順の order を作る（DFS）
		std::vector<int> order;
		order.reserve(n);
		std::vector<char> visited(n, 0);

		auto dfs = [&](auto&& self, int b) -> void
			{
				if (b < 0 || b >= n) return;
				if (visited[b]) return;
				visited[b] = 1;

				order.push_back(b);
				for (int c : children[b])
					self(self, c);
			};

		for (int r : roots)
			dfs(dfs, r);

		// もし到達できていない骨があれば追加（データ破損／循環などの対策）
		for (int i = 0; i < n; i++)
			if (!visited[i])
				dfs(dfs, i);

		// oldToNew を作る
		std::vector<int> oldToNew(n, -1);
		for (int newIndex = 0; newIndex < (int)order.size(); newIndex++)
			oldToNew[order[newIndex]] = newIndex;

		// bones を並び替え
		std::vector<Bone> newBones(n);
		for (int newIndex = 0; newIndex < n; newIndex++)
		{
			int oldIndex = order[newIndex];
			newBones[newIndex] = skeleton.bones[oldIndex];

			int oldParent = newBones[newIndex].parent;
			if (oldParent < 0) newBones[newIndex].parent = -1;
			else			   newBones[newIndex].parent = oldToNew[oldParent];
		}
		skeleton.bones.swap(newBones);

		// boneNameToIndex を再構築
		skeleton.boneNameToIndex.clear();
		skeleton.boneNameToIndex.reserve((size_t)n);
		for (int i = 0; i < n; i++)
			skeleton.boneNameToIndex[skeleton.bones[i].name] = i;

		// boneToNode も再構築
		if ((int)skeleton.boneToNode.size() == n)
		{
			std::vector<int> newBoneToNode(n, -1);
			for (int newIndex = 0; newIndex < n; newIndex++)
			{
				int oldIndex = order[newIndex];
				newBoneToNode[newIndex] = skeleton.boneToNode[oldIndex];
			}
			skeleton.boneToNode.swap(newBoneToNode);
		}

	}

	// メッシュの焼きこみ行列を作る（今は結局使っておらず）

	// 静的メッシュ用の頂点情報を詰め込む
	void FillStaticVertexesFromMesh(const aiMesh* mesh, std::vector<VERTEX_3D>& out, bool flipV)
	{
		out.clear();
		out.resize(mesh->mNumVertices);

		for (unsigned i = 0; i < mesh->mNumVertices; i++)
		{
			auto& vertex = out[i];

			vertex.position = {
				mesh->mVertices[i].x,
				mesh->mVertices[i].y,
				mesh->mVertices[i].z
			};
			if (mesh->HasNormals())
			{
				vertex.normal = Vector3{ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
			}
			else
				vertex.normal = Vector3{ 0, 1, 0 };

			if (mesh->HasTextureCoords(0))
			{
				float u = mesh->mTextureCoords[0][i].x;
				float v = mesh->mTextureCoords[0][i].y;
				if (flipV) v = 1.0f - v;
				vertex.texcoord = { u, v };
			}
			else
				vertex.texcoord = Vector2{ 0, 0 };
			vertex.diffuse = { 1, 1, 1, 1 };
		}
	}

	// aiMaterial から”使えそうなテクスチャパス”を１つとる
	bool TryGetTexturePath(const aiMaterial* mat, aiTextureType type, aiString& outPath)
	{
		if (!mat) return false;
		if (mat->GetTextureCount(type) == 0) return false;
		return (mat->GetTexture(type, 0, &outPath) == AI_SUCCESS);
	}
	bool GetBestTexturePath(aiMaterial* mat, aiString& outPath)
	{
		// まずは DIFFUSE
		if (TryGetTexturePath(mat, aiTextureType_DIFFUSE, outPath)) return true;
		// PBR 系が入っている場合のフォールバック（Assimp のバージョンやFBXにより揺れる）
#ifdef aiTextureType_BASE_COLOR
		if (TryGetTexturePath(mat, aiTextureType_BASE_COLOR, outPath)) return true;
#endif
		// 最後の保険
		if (TryGetTexturePath(mat, aiTextureType_UNKNOWN, outPath)) return true;
		return false;
	}
	std::string NormalizeSlashes(std::string s)
	{
		for (auto& ch : s) if (ch == '\\') ch = '/';
		return s;
	}
	std::string GetFilenameOnly(const std::string& s)
	{
		std::string n = NormalizeSlashes(s);
		size_t p = n.find_last_of('/');
		if (p == std::string::npos) return n;
		return n.substr(p + 1);
	}
	// マップを作る
	void BuildEmbeddedTextureSRVMap(const aiScene* scene, std::unordered_map<std::string, ID3D11ShaderResourceView*>& outMap)
	{
		outMap.clear();
		if (!scene || scene->mNumTextures == 0) return;

		auto* dev = Renderer::Device();
		if (!dev) return;

		using namespace DirectX;

		for (unsigned int i = 0; i < scene->mNumTextures; i++)
		{
			aiTexture* t = scene->mTextures[i];
			if (!t) continue;

			std::string key = NormalizeSlashes(t->mFilename.C_Str());
			if (key.empty())
			{
				// 空なら便宜上のキーを作る
				key = "*embedded_" + std::to_string(i);
			}

			TexMetadata metadata{};
			ScratchImage image{};
			ID3D11ShaderResourceView* srv = nullptr;

			HRESULT hr = E_FAIL;

			if (t->mHeight == 0)
			{
				// 圧縮形式（png/jpg等）が丸ごと入っている：mWidthがバイト数
				const uint8_t* data = reinterpret_cast<const uint8_t*>(t->pcData);
				const size_t size = static_cast<size_t>(t->mWidth);
				hr = LoadFromWICMemory(data, size, WIC_FLAGS_NONE, &metadata, image);
			}
			else
			{
				// 非圧縮（生ピクセル）※ここは形式が色々あるので今回は最低限の対応
				// 多くのFBXはここには来ない（来るなら別途対応が必要）（ひと先ずスキップしてよい）
				continue;
			}
			if (FAILED(hr)) continue;

			hr = CreateShaderResourceView(dev, image.GetImages(), image.GetImageCount(), metadata, &srv);

			if (FAILED(hr) || !srv) continue;

			// フルキーで登録
			outMap[key] = srv;
			// ファイル名だけでも登録（Material側がファイル名だけを返すケース対策）
			outMap[GetFilenameOnly(key)] = srv;
		}
	}
	ID3D11ShaderResourceView* ResolveMaterialSRV(const aiScene* scene,
												 aiMaterial* mat, 
												 const std::filesystem::path& modelDir, 
												 const std::unordered_map<std::string, ID3D11ShaderResourceView*>& embeddedMap)
	{
		if (!scene || !mat) return nullptr;
		aiString texPath;
		if (!GetBestTexturePath(mat, texPath)) return nullptr; // そもそもテクスチャ指定なし

		std::string p = NormalizeSlashes(texPath.C_Str());

		// 埋め込み辞書で引く
		if (!p.empty() && p[0] == '*') // Assimp では埋め込みが "*0" みたいな形式で来ることもある
		{
			// "*0" → scene->mTextures[0]
			int index = atoi(p.c_str() + 1);
			if (index >= 0 && index < (int)scene->mNumTextures)
			{
				std::string key = NormalizeSlashes(scene->mTextures[index]->mFilename.C_Str());
				auto it = embeddedMap.find(key);
				if (it != embeddedMap.end()) return it->second;

				// filename-only fallback
				auto it2 = embeddedMap.find(GetFilenameOnly(key));
				if (it2 != embeddedMap.end()) return it2->second;
			}
		}
		// 通常の文字列でも辞書にあれば使う
		{
			auto it = embeddedMap.find(p);
			if (it != embeddedMap.end()) return it->second;
			auto it2 = embeddedMap.find(GetFilenameOnly(p));
			if (it2 != embeddedMap.end()) return it2->second;
		}

		// ファイルロード
		std::filesystem::path texRel(p);
		std::filesystem::path full = texRel.is_absolute() ? texRel : (modelDir / texRel);

		std::string fullStr = full.generic_string();
		return Texture::LoadAndRegisterKey(fullStr.c_str());
	}
}

void ModelLoader::LoadMeshFromFile(MeshFilterComponent* filter, const std::string& path, std::vector<ID3D11ShaderResourceView*>& outSrvs, bool flipV)
{
	assert(filter);

	// Assimp でモデルを読む
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path.c_str(),
											 aiProcess_Triangulate
											 /*aiProcess_GenNormals |
											 aiProcess_CalcTangentSpace |
											 aiProcess_JoinIdenticalVertices |
											 aiProcess_ImproveCacheLocality |
											 aiProcess_SortByPType*/
											);
	if (!scene || !scene->HasMeshes())
	{
		assert(false && "Assimp load failed or no mesh");
		return;
	}

	// ----- VB/IV 統合 + subMesh 作成 ----- 
	std::vector<VERTEX_3D> vertexes;
	std::vector<uint32_t> indexes;
	unsigned int totalVertexes = 0;
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		totalVertexes += scene->mMeshes[i]->mNumVertices;
	vertexes.reserve(totalVertexes);

	filter->ClearSubMeshes();
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i]; 
		if (!mesh) continue;

		// このメッシュがIBに追加し始める位置（index単位）
		uint32_t indexStart = (uint32_t)indexes.size();

		// 頂点バッファ用の配列を作成
		std::vector<VERTEX_3D> meshVertex;
		FillStaticVertexesFromMesh(mesh, meshVertex, flipV);
		// 今まで積んだ頂点を基準にオフセット
		uint32_t baseVertex = static_cast<uint32_t>(vertexes.size());

		vertexes.insert(vertexes.end(), meshVertex.begin(), meshVertex.end());

		// インデックス配列を作成
		for (unsigned i = 0; i < mesh->mNumFaces; i++)
		{
			const aiFace& face = mesh->mFaces[i];
			assert(face.mNumIndices == 3); // Triangle 済み前提
			indexes.push_back(baseVertex + face.mIndices[0]);
			indexes.push_back(baseVertex + face.mIndices[1]);
			indexes.push_back(baseVertex + face.mIndices[2]);
		}

		uint32_t indexCount = (uint32_t)indexes.size() - indexStart;

		
		SubMesh sm{};
		sm.indexStart = indexStart;
		sm.indexCount = indexCount;
		sm.materialSlot = (int)mesh->mMaterialIndex;
		if (indexCount > 0)
			filter->AddSubMesh(sm);
		
	}

	if (vertexes.empty() || indexes.empty())
	{
		assert(false && "No vertexes or indexes after merging meshes");
		return;
	}
	
	// ----- VB 作成 -----
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(VERTEX_3D) * static_cast<UINT>(vertexes.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = vertexes.data();
	ID3D11Buffer* vb = nullptr;
	HRESULT hr = Renderer::Device()->CreateBuffer(&bd, &sd, &vb);
	if (FAILED(hr))
	{
		assert(false && "VreateBuffer VB failed");
		return;
	}

	// ----- IB 作成 -----
	D3D11_BUFFER_DESC ibd{};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(uint32_t) * static_cast<UINT>(indexes.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA isd{};
	isd.pSysMem = indexes.data();
	ID3D11Buffer* ib = nullptr;
	hr = Renderer::Device()->CreateBuffer(&ibd, &isd, &ib);
	if (FAILED(hr))
	{
		vb->Release();
		assert(false && "CreateBuffer IB failed");
		return;
	}

	// MeshFilter に渡す
	filter->SetVertexBuffer(vb, sizeof(VERTEX_3D), static_cast<UINT>(vertexes.size()), 0, true);
	filter->SetIndexBuffer(ib, static_cast<UINT>(indexes.size()), DXGI_FORMAT_R32_UINT, true);
	filter->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ----- Material slot count -----
	const int slotCount = (scene->mNumMaterials > 0) ? (int)scene->mNumMaterials : 0;
	filter->SetMaterialSlotCount(slotCount);
	outSrvs.clear();
	outSrvs.resize(slotCount, nullptr);

	// ----- Texture の取得 -----
	// モデルファイルのディレクトリを取得
	std::filesystem::path modelPath(path);
	std::filesystem::path modelDir = modelPath.parent_path();

	// embedded texture -> SRV map
	std::unordered_map<std::string, ID3D11ShaderResourceView*> embeddedMap;
	BuildEmbeddedTextureSRVMap(scene, embeddedMap);

	// マテリアルスロットに入れる
	for (int matIndex = 0; matIndex < slotCount; matIndex++)
	{
		aiMaterial* mat = scene->mMaterials[matIndex];
		ID3D11ShaderResourceView* srv = ResolveMaterialSRV(scene, mat, modelDir, embeddedMap);
		outSrvs[matIndex] = srv;
	}
}

void ModelLoader::LoadSkinnedMeshFromFile(MeshFilterComponent* filter, const std::string& path, const Skeleton& skeleton, std::vector<ID3D11ShaderResourceView*>& outSrvs, bool flipV)
{
	assert(filter);

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path,
											 aiProcess_Triangulate 
											 //aiProcess_ConvertToLeftHanded  // これいる？
											 //aiProcess_GenNormals |
											 //aiProcess_CalcTangentSpace |
											 //aiProcess_JoinIdenticalVertices |
											 //aiProcess_ImproveCacheLocality |
											 //aiProcess_SortByPType |
											 //aiProcessPreset_TargetRealtime_MaxQuality
											);
	if (!scene || !scene->HasMeshes())
	{
		assert(false && "Assimp load failed or no mesh");
		return;
	}

	// ----- 全メッシュをまとめて1つの頂点/インデックス配列にする -----
	std::vector<VERTEX_SKINNED> vertexes;
	std::vector<uint32_t> indexes;
	unsigned int totalVertexes = 0;
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		totalVertexes += scene->mMeshes[i]->mNumVertices;
	vertexes.reserve(totalVertexes);

	filter->ClearSubMeshes();
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		if (!mesh) continue;

		// このメッシュがIBに追加し始める位置（index単位）
		uint32_t indexStart = (uint32_t)indexes.size();

		// このメッシュ分の頂点を一時バッファに詰める
		std::vector<VERTEX_SKINNED> meshVertexes;
		FillSkinnedVertexesFromMesh(mesh, skeleton, meshVertexes, flipV);

		// 今まで積んだ頂点を基準にオフセット
		uint32_t baseVertex = static_cast<uint32_t>(vertexes.size());

		// 全体バッファに追加
		vertexes.insert(vertexes.end(), meshVertexes.begin(), meshVertexes.end());

		// インデックス配列もこのメッシュ分を追加
		for (unsigned int j = 0; j < mesh->mNumFaces; j++)
		{
			const aiFace& face = mesh->mFaces[j];
			assert(face.mNumIndices == 3);

			indexes.push_back(baseVertex + face.mIndices[0]);
			indexes.push_back(baseVertex + face.mIndices[1]);
			indexes.push_back(baseVertex + face.mIndices[2]);
		}

		// このメッシュが追加したindex数
		uint32_t indexCount = (uint32_t)indexes.size() - indexStart;

		// SubMesh 登録
		SubMesh sm{};
		sm.indexStart = indexStart;
		sm.indexCount = indexCount;
		sm.materialSlot = (int)mesh->mMaterialIndex;
		// indexCount == 0 の mesh はスキップしてOK
		if (sm.indexCount > 0)
			filter->AddSubMesh(sm);		
	}

	if (vertexes.empty() || indexes.empty())
	{
		assert(false && "No vertexes or indexes after merging meshes");
		return;
	}

	// ----- VB 作成 -----
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(VERTEX_SKINNED) * static_cast<UINT>(vertexes.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = vertexes.data();
	ID3D11Buffer* vb = nullptr;
	HRESULT hr = Renderer::Device()->CreateBuffer(&bd, &sd, &vb);
	if (FAILED(hr))
	{
		assert(false && "VreateBuffer VB failed");
		return;
	}

	// ----- IB 作成 -----
	D3D11_BUFFER_DESC ibd{};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(uint32_t) * static_cast<UINT>(indexes.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA isd{};
	isd.pSysMem = indexes.data();
	ID3D11Buffer* ib = nullptr;
	hr = Renderer::Device()->CreateBuffer(&ibd, &isd, &ib);
	if (FAILED(hr))
	{
		vb->Release();
		assert(false && "CreateBuffer IB failed");
		return;
	}

	// MeshFilter に渡す
	filter->SetVertexBuffer(vb, sizeof(VERTEX_SKINNED), static_cast<UINT>(vertexes.size()), 0, true);
	filter->SetIndexBuffer(ib, static_cast<UINT>(indexes.size()), DXGI_FORMAT_R32_UINT, true);
	filter->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ----- Material slot count -----
	const int slotCount = (scene->mNumMaterials > 0) ? (int)scene->mNumMaterials : 0;
	filter->SetMaterialSlotCount(slotCount);
	outSrvs.clear();
	outSrvs.resize(slotCount, nullptr);

	// ----- Texture の取得 -----
	// モデルファイルのディレクトリを取得
	std::filesystem::path modelPath(path);
	std::filesystem::path modelDir = modelPath.parent_path();

	// embedded texture -> SRV map
	std::unordered_map<std::string, ID3D11ShaderResourceView*> embeddedMap;
	BuildEmbeddedTextureSRVMap(scene, embeddedMap);

	// マテリアルスロットに入れる
	for (int matIndex = 0; matIndex < slotCount; matIndex++)
	{
		aiMaterial* mat = scene->mMaterials[matIndex];
		ID3D11ShaderResourceView* srv = ResolveMaterialSRV(scene, mat, modelDir, embeddedMap);
		outSrvs[matIndex] = srv;
	}
}

AnimationClip ModelLoader::BuildAnimationClip(const aiScene* scene, const Skeleton& skeleton, unsigned int animeIndex)
{
	AnimationClip clip{};

	if (scene->mNumAnimations == 0) return clip; // 空のクリップ
	if (animeIndex >= scene->mNumAnimations) return clip;

	const aiAnimation* pAnime = scene->mAnimations[animeIndex];

	// 名前取得
	clip.name = pAnime->mName.C_Str();

	float ticksPerSec = (pAnime->mTicksPerSecond != 0.0) ? (float)pAnime->mTicksPerSecond : 30.0f; // 適当なデフォルト値（30.0f）

	clip.ticksPerSecond = ticksPerSec;
	clip.duration = (float)(pAnime->mDuration / ticksPerSec); // 秒に変換する

	// 骨名 → index マップ
	const std::unordered_map<std::string, int>& boneNameToIndex = skeleton.boneNameToIndex;

	clip.tracks.reserve(pAnime->mNumChannels);

	for (unsigned int c = 0; c < pAnime->mNumChannels; c++)
	{
		const aiNodeAnim* nodeAnim = pAnime->mChannels[c];
		std::string nodeName = nodeAnim->mNodeName.C_Str();

		auto it = boneNameToIndex.find(nodeName);
		if (it == boneNameToIndex.end()) continue; // Skelton にないノードはスキップ

		BoneTrack track{};
		track.boneIndex = it->second;

		// TranslationKey
		track.translationKeys.reserve(nodeAnim->mNumPositionKeys);
		for (unsigned int i = 0; i < nodeAnim->mNumPositionKeys; i++)
		{
			const auto& k = nodeAnim->mPositionKeys[i];
			TranslationKey key{};
			key.time = (float)(k.mTime / ticksPerSec); // 秒
			key.value = Vector3{
				k.mValue.x,
				k.mValue.y,
				k.mValue.z
			};
			track.translationKeys.push_back(key);
		}
		// RotationKey
		track.rotationKeys.reserve(nodeAnim->mNumRotationKeys);
		for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys; i++)
		{
			const auto& k = nodeAnim->mRotationKeys[i];
			RotationKey key{};
			key.time = (float)(k.mTime / ticksPerSec); // 秒
			key.value = Quaternion{
				k.mValue.x,
				k.mValue.y,
				k.mValue.z,
				k.mValue.w
			};
			track.rotationKeys.push_back(key);
		}
		// ScaleKey
		track.scaleKeys.reserve(nodeAnim->mNumScalingKeys);
		for (unsigned int i = 0; i < nodeAnim->mNumScalingKeys; i++)
		{
			const auto& k = nodeAnim->mScalingKeys[i];
			ScaleKey key{};
			key.time = (float)(k.mTime / ticksPerSec); // 秒
			key.value = Vector3{
				k.mValue.x,
				k.mValue.y,
				k.mValue.z
			};
			track.scaleKeys.push_back(key);
		}
		clip.tracks.push_back(std::move(track));
	}
	return clip;
}

AnimationClip ModelLoader::BuildAnimationClipFromFile(const std::string& path, const Skeleton& skeleton, unsigned int animeIndex)
{
	AnimationClip clip{};

	Assimp::Importer importer;
	//importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	const aiScene* scene = importer.ReadFile(path,
											 aiProcess_Triangulate 
											 //aiProcess_JoinIdenticalVertices |
											 //aiProcess_ConvertToLeftHanded | // これいる？
											 //aiProcess_SortByPType
											 //aiProcessPreset_TargetRealtime_MaxQuality
											);
	if (!scene)
	{
		assert(false && "Assimp load failed");
		return clip; // 中身は空のまま
	}
	if (scene->mNumAnimations == 0)
	{
		// アニメなし
		return clip;
	}
	if (animeIndex >= scene->mNumAnimations)
	{
		assert(false && "animeIndex out of range");
		return clip; // 中身は空のまま
	}

	clip = BuildAnimationClip(scene, skeleton, animeIndex);
	return clip;
}

Skeleton ModelLoader::BuildSkeleton(const aiScene* scene)
{
	Skeleton skeleton{};
	if (!scene || !scene->mRootNode) return skeleton;


	// 全ノードをフラットに構築
	skeleton.meshToNode.resize(scene->mNumMeshes, -1);
	BuildNodesRecursive(scene->mRootNode, -1, skeleton);

	// 重みをもつボーンを収集
	auto boneMap = CollectBones(scene);
	// ボーンが１本もないモデルなら空のまま
	if (boneMap.empty()) return skeleton;

	// boneName -> boneIndex を確定
	for (auto& [name, aiBonePtr] : boneMap)
	{
		Bone b{};
		b.name = name; // 名前格納
		b.invBindPose = ToMatrix4x4(aiBonePtr->mOffsetMatrix); // bindPose の逆行列
		skeleton.boneNameToIndex[name] = (int)skeleton.bones.size(); // 名前から番号を引けるように
		skeleton.bones.push_back(b);
	}
	// boneIndex -> nodeIndex の対応を作る
	skeleton.boneToNode.resize(skeleton.bones.size(), -1);
	for (size_t i = 0; i < skeleton.bones.size(); i++)
	{
		const std::string& boneName = skeleton.bones[i].name; // ボーンの名前

		auto it = skeleton.nodeNameToIndex.find(boneName);
		if (it == skeleton.nodeNameToIndex.end())
		{
			// 見つからない処理
			printf("[BuildSkeleton] bone node not found: %s\n", boneName.c_str());
			skeleton.boneToNode[i] = -1;
			continue;
		}

		skeleton.boneToNode[i] = it->second;
	}
	// 親ボーン解決
	for (size_t i = 0; i < skeleton.bones.size(); i++)
	{
		int nodeIndex = skeleton.boneToNode[i];
		Bone& bone = skeleton.bones[i];

		if (nodeIndex < 0) // ボーン名に対応するノードが見つからなかった場合の処理（回避策）
		{
			bone.parent = -1;
			bone.bindLocal.identity();
			continue;
		}

		bone.parent = FindParentBoneIndexByNode(skeleton, nodeIndex);

		// bindLocal は「骨ノードのローカル」
		bone.bindLocal = skeleton.nodes[nodeIndex].bindLocal;
		ExtractBindTRS(bone.bindLocal, bone.bindT, bone.bindR, bone.bindS);
	}

	skeleton.rootBone = -1;
	for (int i = 0; i < (int)skeleton.bones.size(); i++)
	{
		if (skeleton.bones[i].parent == -1)
		{
			skeleton.rootBone = i;
			break;
		}
	}
	if (skeleton.rootBone < 0)
	{
		skeleton.rootBone = 0;
	}

	// ボーン配列をソートする
	SortBonesParentFirst(skeleton);
	return skeleton;
}

Skeleton ModelLoader::BuildSkeletonFromFile(const std::string& path)
{
	Skeleton skeleton{};
	Assimp::Importer importer;
	//importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	const aiScene* scene = importer.ReadFile(path,
											 aiProcess_Triangulate 
											 //aiProcess_GenNormals |
											 //aiProcess_ConvertToLeftHanded | // これいる？
											 //aiProcess_CalcTangentSpace |
											 //aiProcess_JoinIdenticalVertices |
											 //aiProcess_ImproveCacheLocality |
											 //aiProcess_SortByPType
											);
	if (!scene || !scene->HasMeshes())
	{
		assert(false && "Assimp load failed or no mesh");
		return skeleton; // 中身は空のまま
	}

	skeleton = BuildSkeleton(scene);
	return skeleton;
}
