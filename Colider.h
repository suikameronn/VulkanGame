#pragma once

#include<map>
#include<array>

#include"Object.h"
#include"GltfModel.h"

#include"GpuBufferFactory.h"
#include"DescriptorSetFactory.h"

#include"MeshRendererComp.h"

#include<random>

struct SimplexVertex
{
	//ミンコフスキー空間上の座標
	glm::vec3 point;

	//上の座標が割り出されるコライダーの頂点のペアの一対
	glm::vec3 myPoint;

	//相手のコライダーのペアの一対
	glm::vec3 oppPoint;

	SimplexVertex()
	{
		point = glm::vec3(0.0f);

		myPoint = glm::vec3(0.0f);

		oppPoint = glm::vec3(0.0f);
	}
};

//GJKの構造体
struct Simplex
{
private:
	//GJK法で扱う最大4つの頂点を持つ図形
	std::array<SimplexVertex, 4> vertices;

public:
	//有効な頂点の数
	int size;

	Simplex()
	{
		size = 0;
		glm::vec3 zero = glm::vec3(0.0f, 0.0f, 0.0f);
		vertices = {};
	}

	//頂点の追加
	void push_front(const SimplexVertex& v)
	{
		vertices = { v,vertices[0],vertices[1],vertices[2] };

		size = std::min(size + 1, 4);
	}

	Simplex& operator=(std::initializer_list<SimplexVertex> list)
	{
		size = 0;

		for (SimplexVertex point : list)
			vertices[size++] = point;

		return *this;
	}

	SimplexVertex& operator[](int i) { return vertices[i]; }

	//epa法での構造へのコピー
	void setSimplexVertices(std::vector<SimplexVertex>& polytope)
	{
		polytope.resize(vertices.size());
		std::copy(vertices.begin(), vertices.end(), polytope.begin());
	}
};

//当たり判定用のクラス
class Colider
{
private:

	//コライダーが当たり判定を解消するかどうか
	bool trigger;

	//コライダー自身のオフセット
	glm::vec3 offsetPos;
	glm::vec3 offsetScale;

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

	//コライダー当たり判定用用のインデックス
	std::vector<uint32_t> coliderIndices;
	//描画用のインデックス
	std::vector<uint32_t> drawColiderIndices;
	//SAT利用時のインデックス
	std::vector<uint32_t> satIndices;

	//コライダー描画用のファクトリー
	std::shared_ptr<GpuBufferFactory> bufferFactory;
	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;
	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;

	//コライダー描画用の変数
	std::shared_ptr<GpuBuffer> vertBuffer;
	std::shared_ptr<GpuBuffer> indeBuffer;
	std::shared_ptr<GpuBuffer> matBuffer;
	std::shared_ptr<DescriptorSet> descriptorSet;

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
	SimplexVertex getSupportVector(const std::unique_ptr<Colider>& oppColider, glm::vec3 dir);
	//GJKの次の単体を求める
	bool nextSimplex(Simplex& simplex, glm::vec3& dir);
	//その線分から最も近い頂点を求める
	glm::vec3 getClosestLineToVertex(glm::vec3 lineStart, glm::vec3 lineFinish, glm::vec3 point);
	//GJK法での当たり判定を実行
	bool GJK(const std::unique_ptr<Colider>& oppColider,glm::vec3& collisionDepthVec, glm::vec3& myCollisionPoint, glm::vec3& oppCollisionPoint);
	//GJK法後にEPA法で衝突を解消するためのベクトルを取得
	void EPA(const std::unique_ptr<Colider>& oppColider, Simplex& simplex, glm::vec3& collisionDepthVec, glm::vec3& myCollisionPoint, glm::vec3& oppCollisionPoint);
	//衝突点を計算する
	void CollisionPoint(const std::array<SimplexVertex, 3>& triangle, const glm::vec3& point, glm::vec3& myCollisionPoint, glm::vec3& oppCollisionPoint);
	//三角形上の頂点の重心座標を求める
	std::array<float, 3> CenterCoord(const std::array<glm::vec3, 3>& triangle, const glm::vec3& position);
	//分離軸定理を利用した当たり判定を実行、衝突を解消するためのベクトルも計算
	bool SAT(const std::unique_ptr<Colider>& oppColider, float& collisionDepth, glm::vec3& collisionNormal);
	//同一の線分を含まなければその頂点を単体に含める
	void addIfUniqueEdge(std::vector<std::pair<int, int>>& edges, const std::vector<int>& faces,
						 int a, int b);
	//面の法線を取得
	std::pair<std::vector<glm::vec4>, int> getFaceNormals(
		std::vector<SimplexVertex>& vertices,
		std::vector<int>& faces);

	//gltfモデルから頂点を取得する
	void setVertices(GltfNode* node, int& loadedVertexCount, int& loadedIndexCount);

public:

	Colider(std::shared_ptr<GltfModel> model,bool isTrigger
		, std::shared_ptr<GpuBufferFactory> buffer, std::shared_ptr<DescriptorSetLayoutFactory> layout
		, std::shared_ptr<DescriptorSetFactory> desc);

	bool isTrigger()
	{
		return trigger;
	}

	//描画に必要なリソースを作成する
	void createBuffer();
	void createDescriptorSet();

	//コライダーの各オフセットを設定する
	void setOffsetPos(const glm::vec3& pos);
	void setOffsetScale(const glm::vec3& scale);

	//Modelクラスの初期座標から座標変換を適用する
	void initFrameSettings(glm::vec3 initScale);
	//コライダーのスケールを設定
	glm::vec3 scale;
	//コライダーのスケール行列を取得
	glm::mat4 getScaleMat();
	//Modelクラスの移動などをコライダーにも反映
	void reflectMovement(const glm::vec3& translate, const glm::mat4& rotate, const glm::vec3& scale);
	//重心を返す
	glm::vec3 getCenterPos();

	//サポート写像を求める(SAT用)
	void projection(float& min, float& max, glm::vec3& minVertex, glm::vec3& maxVertex, glm::vec3& axis);
	//descriptorSetの設定
	void setDescriptorSet(VkDescriptorSet descriptorSet);
	
	//コライダーのレンダリング用の変数
	std::shared_ptr<GpuBuffer> getVertexBuffer() { return vertBuffer; }
	std::shared_ptr<GpuBuffer> getIndexBuffer() { return indeBuffer; }
	std::shared_ptr<DescriptorSet> getDescriptorSet() { return descriptorSet; }

	//SAT用当たり判定の実行
	virtual bool Intersect(const std::unique_ptr<Colider>& oppColider, glm::vec3& collisionVector, glm::vec3& myCollisionPoint, glm::vec3& oppCollisionPoint);
	//GJK用当たり判定の実行
	virtual bool Intersect(const std::unique_ptr<Colider>& oppColider);
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
	//描画用のインデックスのサイズを取得
	int getDrawColiderIndicesSize() { return static_cast<int>(drawColiderIndices.size()); }
	//描画用のインデックスを取得
	uint32_t* getDrawColiderIndices() { return drawColiderIndices.data(); }
};