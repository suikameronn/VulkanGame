#pragma once

#include<map>
#include<array>

#include"Object.h"
#include"GltfModel.h"
#include"EnumList.h"

#include<random>

//GJKの構造体
struct Simplex
{
private:
	//GJK法で扱う最大4つの頂点を持つ図形
	std::array<glm::vec3, 4> points;

public:
	//有効な頂点の数
	int size;

	Simplex()
	{
		size = 0;
		glm::vec3 zero = glm::vec3(0.0f, 0.0f, 0.0f);
		points = { zero };
	}

	//頂点の追加
	void push_front(glm::vec3 v)
	{
		points = { v,points[0],points[1] ,points[2] };
		size = std::min(size + 1, 4);
	}

	Simplex& operator=(std::initializer_list<glm::vec3> list)
	{
		size = 0;

		for (glm::vec3 point : list)
			points[size++] = point;

		return *this;
	}

	glm::vec3& operator[](int i) { return points[i]; }

	//epa法での構造へのコピー
	void setSimplexVertices(std::vector<glm::vec3>& polytope)
	{
		polytope.resize(points.size());
		std::copy(points.begin(), points.end(), polytope.begin());
	}
};

//当たり判定用のクラス
class Colider
{
private:

	//AABB上の左上、右下の頂点
	glm::vec3 min, max;
	//AABBに座標変換を加えたもの
	glm::vec3 transformedMin, transformedMax;

	//拡大行列
	glm::mat4 scaleMat;
	
	//現在のコライダーのAABBの頂点
	std::vector<glm::vec3> coliderVertices;
	//座標変換を加える前のコライダーの頂点の座標
	std::vector<glm::vec3> originalVertexPos;

	//コライダー描画用のインデックス
	std::vector<uint32_t> coliderIndices;
	//SAT利用時のインデックス
	std::vector<uint32_t> satIndices;

	//コライダー描画用の変数
	BufferObject pointBuffer;
	MappedBuffer mappedBuffer;
	DescriptorInfo descInfo;
	DescSetData descSetData;

	//引数の方向ベクトルのほうにその頂点があるかどうか
	bool sameDirection(glm::vec3 dir, glm::vec3 point) { return glm::dot(dir, point) > 0.0f; }

	//線分同士でわかる範囲でミンコフスキー差が原点を含みそうか調べる
	bool Line(Simplex& simplex,glm::vec3& dir);
	//三角形でわかる範囲でミンコフスキー差が原点を含みそうか調べる
	bool Triangle(Simplex& simplex, glm::vec3& dir);
	//四角錐でわかる範囲でミンコフスキー差が原点を含むか調べる
	bool Tetrahedron(Simplex& simplex, glm::vec3& dir);

	//引数の方向ベクトルの向きで最も遠い頂点を求める
	glm::vec3 getFurthestPoint(glm::vec3 dir);
	//サポート写像を求める
	glm::vec3 getSupportVector(std::shared_ptr<Colider> oppColider, glm::vec3 dir);
	//GJKの次の単体を求める
	bool nextSimplex(Simplex& simplex, glm::vec3& dir);
	//その線分から最も近い頂点を求める
	glm::vec3 getClosestLineToVertex(glm::vec3 lineStart, glm::vec3 lineFinish, glm::vec3 point);
	//GJK法での当たり判定を実行
	bool GJK(std::shared_ptr<Colider> oppColider,glm::vec3& collisionDepthVec);
	//GJK法後にEPA法で衝突を解消するためのベクトルを取得
	void EPA(std::shared_ptr<Colider> oppColider, Simplex& simplex, glm::vec3& collisionDepthVec);
	//分離軸定理を利用した当たり判定を実行、衝突を解消するためのベクトルも計算
	bool SAT(std::shared_ptr<Colider> oppColider, float& collisionDepth, glm::vec3& collisionNormal);
	//同一の線分を含まなければその頂点を単体に含める
	void addIfUniqueEdge(std::vector<std::pair<size_t, size_t>>& edges, const std::vector<size_t>& faces,
						 size_t a, size_t b);
	//面の法線を取得
	std::pair<std::vector<glm::vec4>, size_t> getFaceNormals(
		std::vector<glm::vec3>& vertices,
		std::vector<size_t>& faces);

	//gltfモデルから頂点を取得する
	void setVertices(GltfNode* node, int& loadedVertexCount, int& loadedIndexCount);

public:

	Colider(std::shared_ptr<GltfModel> model);

	//Modelクラスの初期座標から座標変換を適用する
	void initFrameSettings();
	//コライダーのスケールを設定
	glm::vec3 scale;
	//コライダーのスケール行列を取得
	glm::mat4 getScaleMat();
	//Modelクラスの移動などをコライダーにも反映
	void reflectMovement(glm::mat4& transform);

	//サポート写像を求める(SAT用)
	void projection(float& min, float& max, glm::vec3& minVertex, glm::vec3& maxVertex, glm::vec3& axis);
	//descriptorSetの設定
	void setDescriptorSet(VkDescriptorSet descriptorSet);
	
	//コライダーのレンダリング用の変数
	BufferObject* getPointBufferData();
	MappedBuffer* getMappedBufferData();
	DescriptorInfo& getDescInfo();
	DescSetData& getDescSetData() { return descSetData; }

	//SAT用当たり判定の実行
	virtual bool Intersect(std::shared_ptr<Colider> oppColider, glm::vec3& collisionVector);
	//GJK用当たり判定の実行
	virtual bool Intersect(std::shared_ptr<Colider> oppColider);
	//ボックスレイキャスト用の当たり判定の実行
	virtual bool Intersect(glm::vec3 origin, glm::vec3 dir, float length,glm::vec3& normal);

	//座標変換を加えていないコライダーの頂点を取得
	glm::vec3* getColiderOriginalVertices();
	//座標変換を加えたコライダーの頂点を取得
	glm::vec3* getColiderVertices();
	//コライダーの頂点を取得
	int getColiderVerticesSize();
	//コライダーの頂点のインデックスを取得
	int* getColiderIndices();
	//コライダーの頂点のインデックスのサイズを取得
	int getColiderIndicesSize();
	//コライダー用のgpu上のバッファの破棄
	void cleanupVulkan();
};

//メッシュ単位でのコライダー
class MeshColiderNode :public Colider
{
private:
	
	int skinIndex;
	int meshIndex;


	//座標変換前のAABB
	glm::vec3 srcMin, srcMax;

public:

	void reflectMovement(glm::mat4& transform, std::vector<std::array<glm::mat4, 128>>& jointMatrices);

};