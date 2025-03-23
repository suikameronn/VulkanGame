#include"Colider.h"

#include"VulkanBase.h"

Colider::Colider(bool isMesh,int vertexNum,int indexNum,glm::vec3 min, glm::vec3 max)
{
	this->min = min;
	this->max = max;

	this->transformedMin = min;
	this->transformedMax = max;

	scale = glm::vec3(1.0f);
	scaleMat = glm::scale(scale);

	originalVertexPos.resize(8);//座標変換前の座標配列

	originalVertexPos[0] = glm::vec3(min.x, min.y, min.z);
	originalVertexPos[1] = glm::vec3(max.x, min.y, min.z);
	originalVertexPos[2] = glm::vec3(max.x, min.y, max.z);
	originalVertexPos[3] = glm::vec3(min.x, min.y, max.z);
	originalVertexPos[4] = glm::vec3(min.x, max.y, min.z);
	originalVertexPos[5] = glm::vec3(max.x, max.y, min.z);
	originalVertexPos[6] = glm::vec3(max.x, max.y, max.z);
	originalVertexPos[7] = glm::vec3(min.x, max.y, max.z);

	coliderIndices.resize(12);

	coliderIndices = { 1,0,1,2,2,3,3,0,0,4,1,5,2,6,3,7,4,5,5,6,6,7,7,4 };//描画用のインデックス配列

	satIndices.resize(3 * 6);
	satIndices = { 0,4,5,1,5,6,6,2,3,3,7,4,2,1,0,6,5,4 };
	satIndices = { 1,0,2,4,5,6,5,6,1,4,0,7,5,4,1,7,6,3 };//衝突判定用のインデックス配列

	descSetData.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

	isMeshColider = isMesh;
	if (isMeshColider)
	{
		initialVertcesPos.resize(vertexNum);
		transformedVerticesPos.resize(vertexNum);
		meshColiderVertexOptions.resize(vertexNum);
		meshColiderIndices.resize(indexNum);
	}
}

//Modelクラスの初期座標から座標変換を適用する
void Colider::initFrameSettings()
{
	scaleMat = glm::scale(scale);

	for (int i = 0; i < originalVertexPos.size(); i++)
	{
		originalVertexPos[i] = scaleMat * glm::vec4(originalVertexPos[i],1.0);
	}

	coliderVertices.resize(8);
	std::copy(originalVertexPos.begin(), originalVertexPos.end(), coliderVertices.begin());
}

//座標変換を加えたコライダーの頂点を取得
glm::vec3* Colider::getColiderVertices()
{
	return coliderVertices.data();
}

//コライダーの頂点を取得
glm::vec3* Colider::getColiderOriginalVertices()
{
	return originalVertexPos.data();
}

//コライダーの頂点を取得
int Colider::getColiderVerticesSize()
{
	return static_cast<int>(originalVertexPos.size());
}

//コライダーの頂点のインデックスを取得
int* Colider::getColiderIndices()
{
	return (int*)coliderIndices.data();
}

//コライダーの頂点のインデックスのサイズを取得
int Colider::getColiderIndicesSize()
{
	return static_cast<int>(coliderIndices.size());
}

//コライダーのスケール行列を取得
glm::mat4 Colider::getScaleMat()
{
	return scaleMat;
}

//Modelクラスの移動などをコライダーにも反映
void Colider::reflectMovement(glm::mat4& transform)
{
	for (int i = 0; i < coliderVertices.size(); i++)
	{
		coliderVertices[i] = transform * glm::vec4(originalVertexPos[i], 1.0f);
		transformedMin = transform * glm::vec4(min, 1.0f);
		transformedMax = transform * glm::vec4(max, 1.0f);
	}

	if (isMeshColider)
	{
		calcTransformedVertices(transform);
	}
}

//Modelクラスの移動などをコライダーにも反映
void Colider::reflectMovement(glm::mat4& transform,std::vector<std::array<glm::mat4,128>>& animationMatrix)
{
	//AABBの更新
	for (int i = 0; i < coliderVertices.size(); i++)
	{
		coliderVertices[i] = transform * glm::vec4(originalVertexPos[i], 1.0f);
		transformedMin = transform * glm::vec4(min, 1.0f);
		transformedMax = transform * glm::vec4(max, 1.0f);
	}

	if (isMeshColider)
	{//メッシュコライダーの場合、メッシュの更新
		calcTransformedVertices(transform,animationMatrix);
	}
}

//SAT用当たり判定の実行
bool Colider::Intersect(std::shared_ptr<Colider> oppColider, glm::vec3& collisionVector)
{
	float collisionDepth;
	bool collision = false;
	
	if (!isMeshColider || !oppColider->isMeshColider)
	{
		collision = GJK(oppColider, collisionVector);
	}

	if (collision || true)
	{
		if (isMeshColider || oppColider->isMeshColider)
		{
			collision = meshIntersect(oppColider, collisionVector);

			if (collision)
			{
				std::cout << "AA" << std::endl;
			}
		}
	}

	/*
	collision = SAT(oppColider, collisionDepth, collisionVector);
	collisionVector = collisionVector * collisionDepth;
	*/

	return collision;
}

//GJK用当たり判定の実行
bool Colider::Intersect(std::shared_ptr<Colider> oppColider)
{
	float collisionDepth;
	bool collision = false;

	glm::vec3 collisionVector = glm::vec3(0.0f);

	collision = GJK(oppColider, collisionVector);

	/*
	collision = SAT(oppColider, collisionDepth, collisionVector);
	collisionVector = collisionVector * collisionDepth;
	*/

	return collision;
}

//ボックスレイキャスト用の当たり判定の実行
bool Colider::Intersect(glm::vec3 origin, glm::vec3 dir, float length)
{
	glm::vec3 endPoint = origin + dir * length;

	if (endPoint.x >= transformedMin.x && endPoint.x <= transformedMax.x
		|| endPoint.x <= transformedMin.x && endPoint.x >= transformedMax.x)
	{
		if (endPoint.y >= transformedMin.y && endPoint.y <= transformedMax.y
			|| endPoint.y <= transformedMin.y && endPoint.y >= transformedMax.y)
		{
			if (endPoint.z >= transformedMin.z && endPoint.z <= transformedMax.z
				|| endPoint.z <= transformedMin.z && endPoint.z >= transformedMax.z)
			{
				return true;
			}
		}
	}

	return false;
}

//分離軸定理を利用した当たり判定を実行、衝突を解消するためのベクトルも計算
bool Colider::SAT(std::shared_ptr<Colider> oppColider, float& collisionDepth, glm::vec3& collisionNormal)
{
	std::array<glm::vec3, 12> normals;

	collisionDepth = FLT_MAX;
	collisionNormal = { 0.0f,0.0f,0.0f };

	glm::vec3* oppVertices = oppColider->getColiderVertices();

	for (int i = 0; i < 18; i += 3)
	{
		normals[i / 3] = glm::normalize(glm::cross(coliderVertices[satIndices[i + 1]] - coliderVertices[satIndices[i]], coliderVertices[satIndices[i + 2]] - coliderVertices[satIndices[i]]));
	}
	for (int i = 0; i < 18; i += 3)
	{
		normals[(i / 3) + 6] = glm::normalize(glm::cross(oppVertices[satIndices[i + 1]] - oppVertices[satIndices[i]], oppVertices[satIndices[i + 2]] - oppVertices[satIndices[i]]));
	}

	float depthMin = 0, depthMax = 0;

	float min, max, oppMin, oppMax;
	glm::vec3 minVertex, maxVertex, oppMinVertex, oppMaxVertex;
	for (int i = 0; i < 12; i++)
	{
		this->projection(min, max, minVertex, maxVertex, normals[i]);
		oppColider->projection(oppMin, oppMax, oppMinVertex, oppMaxVertex, normals[i]);

		if (!((min <= oppMin && oppMin <= max) || (oppMin <= min && min <= oppMax)))
		{
			return false;
		}
		else
		{
			float tmp = oppMin - max;

			if (abs(collisionDepth) > abs(tmp))
			{
				if (normals[i] != glm::vec3(0.0f))
				{
					depthMin = oppMin;
					depthMax = max;
					collisionDepth = oppMin - max;
					collisionNormal = normals[i];
				}
			}
		}
	}

	return true;
}

//サポート写像を求める(SAT用)
void Colider::projection(float& min, float& max, glm::vec3& minVertex, glm::vec3& maxVertex, glm::vec3& axis)
{
	min = glm::dot(this->coliderVertices[0], axis);
	minVertex = this->coliderVertices[0];
	max = min;

	float tmp;
	for (int i = 1; i < this->getColiderVerticesSize(); i++)
	{
		tmp = glm::dot(this->coliderVertices[i], axis);
		if (tmp > max)
		{
			max = tmp;
			maxVertex = this->coliderVertices[i];
		}
		else if (tmp < min)
		{
			min = tmp;
			minVertex = this->coliderVertices[i];
		}
	}
}

//GJK法での当たり判定を実行
bool Colider::GJK(std::shared_ptr<Colider> oppColider,glm::vec3& collisionDepthVec)
{
	glm::vec3 support = getSupportVector(oppColider, glm::vec3(1.0f, 0.0f, 0.0f));//適当な方向のサポート写像を求める

	Simplex simplex;
	simplex.push_front(support);

	glm::vec3 dir = -support;

	int count = 50;

	while (count > 0)//検査回数に制限を設ける
	{
		support = getSupportVector(oppColider, dir);//サポート写像を求める

		if (glm::dot(support, dir) <= 0.0f)//もし求めたサポート写像が原点の方向と向きが逆だったら、当たり判定を終了
		{
			return false;
		}

		simplex.push_front(support);//ベクトルを追加

		if (nextSimplex(simplex, dir))//単体の更新、四角錐内に原点が含まれていたら、EPAに移行
		{
			EPA(oppColider,simplex, collisionDepthVec);//衝突を解消するためのベクトルの計算
			return true;
		}

		count--;
	}

	return false;
}

//引数の方向ベクトルの向きで最も遠い頂点を求める
glm::vec3 Colider::getFurthestPoint(glm::vec3 dir)
{
	glm::vec3  maxPoint = glm::vec3(0.0f);
	float maxDistance = -FLT_MAX;

	for (int i = 0; i < getColiderVerticesSize(); i++)
	{
		float distance = glm::dot(coliderVertices[i], dir);
		if (distance > maxDistance) {
			maxDistance = distance;
			maxPoint = coliderVertices[i];
		}
	}

	return maxPoint;
}

//サポート写像を求める
glm::vec3 Colider::getSupportVector(std::shared_ptr<Colider> oppColider, glm::vec3 dir)
{
	return getFurthestPoint(dir) - oppColider->getFurthestPoint(-dir);
}

//線分同士でわかる範囲でミンコフスキー差が原点を含みそうか調べる
bool Colider::Line(Simplex& simplex, glm::vec3& dir)
{
	glm::vec3 a = simplex[0];
	glm::vec3 b = simplex[1];

	glm::vec3 ab = b - a;
	glm::vec3 ao = -a;//a0:原点方向のベクトル

	if (sameDirection(ab, ao))//bが原点方向に位置しない=単体を三角形にしても原点を含まない
	{		
		dir = glm::cross(glm::cross(ab, ao), ab);
	}
	else
	{
		simplex = { a };
		dir = ao;
	}

	return false;
}

//三角形でわかる範囲でミンコフスキー差が原点を含みそうか調べる
bool Colider::Triangle(Simplex& simplex, glm::vec3& dir)
{
	glm::vec3 a = simplex[0];
	glm::vec3 b = simplex[1];
	glm::vec3 c = simplex[2];

	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 point = -a;

	glm::vec3 triangleCross = glm::cross(ab, ac);

	if (sameDirection(glm::cross(triangleCross, ac), point))
	{
		if (sameDirection(ac, point))
		{
			simplex = { a,c };
			dir = glm::cross(glm::cross(ac, point), ac);
		}
		else
		{

			simplex = { a,b };
			return Line(simplex, dir);
		}
	}
	else
	{

		if (sameDirection(glm::cross(ab, triangleCross), point))
		{
			simplex = { a,b };
			return Line(simplex, dir);
		}
		else
		{
			if (sameDirection(triangleCross, point))
			{
				dir = triangleCross;
			}
			else
			{
				simplex = { a,c,b };
				dir = -triangleCross;
			}
		}
	}


	return false;
}

//四角錐でわかる範囲でミンコフスキー差が原点を含むか調べる
bool Colider::Tetrahedron(Simplex& simplex, glm::vec3& dir)
{
	glm::vec3 a = simplex[0];
	glm::vec3 b = simplex[1];
	glm::vec3 c = simplex[2];
	glm::vec3 d = simplex[3];

	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 ad = d - a;
	glm::vec3 point = -a;

	glm::vec3 abc = glm::cross(ab, ac);
	glm::vec3 acd = glm::cross(ac, ad);
	glm::vec3 adb = glm::cross(ad, ab);

	if (sameDirection(abc, point))
	{
		simplex = { a,b,c };
		return Triangle(simplex, dir);
	}

	if (sameDirection(acd, point))
	{
		simplex = { a,c,d };
		return Triangle(simplex, dir);
	}

	if (sameDirection(adb, point))
	{
		simplex = { a,d,b };
		return Triangle(simplex, dir);
	}

	return true;
}

//GJKの次の単体を求める
bool Colider::nextSimplex(Simplex& simplex, glm::vec3& dir)
{
	switch (simplex.size) {
	case 2: return Line(simplex, dir);
	case 3: return Triangle(simplex, dir);
	case 4: return Tetrahedron(simplex, dir);
	}

	return false;
}

//GJK法後にEPA法で衝突を解消するためのベクトルを取得
void Colider::EPA(std::shared_ptr<Colider> oppColider,Simplex& simplex, glm::vec3& collisionDepthVec)
{
	std::vector<glm::vec3> polytope;
	simplex.setSimplexVertices(polytope);

	std::vector<size_t> faces =
	{
		0,1,2,
		0,3,1,
		0,2,3,
		1,3,2
	};

	auto[normals,minFace] = getFaceNormals(polytope, faces);

	glm::vec3 minNormal;
	float minDistance = FLT_MAX;
	int limit = 100;
	while (minDistance == FLT_MAX && limit > 0)
	{
		minNormal = normals[minFace];
		minDistance = normals[minFace].w;

		glm::vec3 support = getSupportVector(oppColider, minNormal);
		float sDistance = glm::dot(minNormal, support);

		if (abs(sDistance - minDistance) > 0.001f)
		{
			minDistance = FLT_MAX;
			std::vector<std::pair<size_t, size_t>> uniqueEdges;
			for (size_t i = 0; i < normals.size(); i++)
			{
				if (sameDirection(normals[i], support))
				{
					size_t f = i * 3;

					addIfUniqueEdge(uniqueEdges, faces, f, f + 1);
					addIfUniqueEdge(uniqueEdges, faces, f + 1, f + 2);
					addIfUniqueEdge(uniqueEdges, faces, f + 2, f);

					faces[f + 2] = faces.back();
					faces.pop_back();
					faces[f + 1] = faces.back();
					faces.pop_back();
					faces[f] = faces.back();
					faces.pop_back();

					normals[i] = normals.back();
					normals.pop_back();

					i--;
				}
			}

			std::vector<size_t> newFaces;
			for (int i = 0; i < uniqueEdges.size(); i++)
			{
				newFaces.push_back(uniqueEdges[i].first);
				newFaces.push_back(uniqueEdges[i].second);
				newFaces.push_back(polytope.size());
			}
			polytope.push_back(support);

			auto[newNormals,newMinFace] = getFaceNormals(polytope, newFaces);

			float oldMinDistance = FLT_MAX;
			for (size_t i = 0; i < normals.size(); i++)
			{
				if (normals[i].w < oldMinDistance)
				{
					oldMinDistance = normals[i].w;
					minFace = i;
				}
			}
			
			if (newNormals[newMinFace].w < oldMinDistance)
			{
				minFace = newMinFace + normals.size();
			}

			faces.insert(faces.end(), newFaces.begin(), newFaces.end());
			normals.insert(normals.end(), newNormals.begin(), newNormals.end());

		}

		limit--;
	}

	collisionDepthVec = minNormal * (minDistance + 0.001f);//遊びをとっておく
}

//同一の線分を含まなければその頂点を単体に含める
void Colider::addIfUniqueEdge(
	std::vector<std::pair<size_t, size_t>>& edges,
	const std::vector<size_t>& faces,
	size_t a,
	size_t b)
{
	auto reverse = std::find(
		edges.begin(),
		edges.end(),
		std::make_pair(faces[b], faces[a])
	);

	if (reverse != edges.end()) {
		edges.erase(reverse);
	}

	else {
		edges.emplace_back(faces[a], faces[b]);
	}
}

//面の法線を取得
std::pair<std::vector<glm::vec4>, size_t> Colider::getFaceNormals(
	std::vector<glm::vec3>& vertices,
	std::vector<size_t>& faces)
{
	std::vector<glm::vec4> normals;
	size_t minTriangle = 0;
	float  minDistance = FLT_MAX;

	for (size_t i = 0; i < faces.size(); i += 3) {
		glm::vec3 a = vertices[faces[i]];
		glm::vec3 b = vertices[faces[i + 1]];
		glm::vec3 c = vertices[faces[i + 2]];

		glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
		float distance = glm::dot(normal, a);

		if (distance < 0) {
			normal *= -1;
			distance *= -1;
		}

		normals.emplace_back(normal, distance);

		if (distance < minDistance) {
			minTriangle = i / 3;
			minDistance = distance;
		}
	}

	return { normals, minTriangle };
}

void Colider::setDescriptorSet(VkDescriptorSet descriptorSet)
{
	descSetData.descriptorSet = descriptorSet;
}

BufferObject* Colider::getPointBufferData()
{
	return &pointBuffer;
}

MappedBuffer* Colider::getMappedBufferData()
{
	return &mappedBuffer;
}

DescriptorInfo& Colider::getDescInfo()
{
	return descInfo;
}

//コライダー用のgpu上のバッファの破棄
void Colider::cleanupVulkan()
{
	VkDevice device = VulkanBase::GetInstance()->GetDevice();

	vkDestroyBuffer(device, pointBuffer.vertBuffer, nullptr);
	vkFreeMemory(device, pointBuffer.vertHandler, nullptr);

	vkDestroyBuffer(device, pointBuffer.indeBuffer, nullptr);
	vkFreeMemory(device, pointBuffer.indeHandler, nullptr);

	vkDestroyBuffer(device, mappedBuffer.uniformBuffer, nullptr);
	vkFreeMemory(device, mappedBuffer.uniformBufferMemory, nullptr);
	mappedBuffer.uniformBufferMapped = nullptr;
}

glm::vec3 Colider::getClosestLineToVertex(glm::vec3 lineStart, glm::vec3 lineFinish, glm::vec3 point)
{
	glm::vec3 lineVector = glm::normalize(lineFinish - lineStart);

	float dot = glm::dot(point, lineVector);

	return lineStart + dot * lineVector;
}

/*メッシュコライダー用の関数*/////////////////////////////////////////////////////////////////////////////////////

//メッシュコライダーのインデックスを取得、メッシュではない場合は、コライダー描画用のインデックスを渡す
std::vector<uint32_t>& Colider::getMeshColiderIndices()
{
	if (isMeshColider)
	{
		return meshColiderIndices;
	}

	//メッシュでないコライダーの場合
	//コライダーの描画用のインデックスを渡す
	return coliderIndices;
}

//メッシュコライダーの当たり判定時に自身のコライダーのタイプに合わせた頂点配列を渡す
std::vector<glm::vec3>& Colider::getMeshColiderVertices()
{
	if (isMeshColider)
	{
		return transformedVerticesPos;
	}

	return coliderVertices;
}

//gltfModelの初期の座標の頂点をコライダーにコピーする
void Colider::setMeshColider(std::shared_ptr<GltfModel> gltfModel)
{
	if (!isMeshColider)
	{
		return;
	}

	gltfModel->setModelVertexIndex(initialVertcesPos,meshColiderVertexOptions, meshColiderIndices);

	std::copy(initialVertcesPos.begin(), initialVertcesPos.end(), transformedVerticesPos.begin());
}

//コライダーの頂点に座標変換を加える(アニメーションはなし)
void Colider::calcTransformedVertices(glm::mat4& transform)
{
	for (int i = 0;i < initialVertcesPos.size();i++)
	{
		transformedVerticesPos[i] = glm::vec3(transform * glm::vec4(initialVertcesPos[i], 1.0f));
	}
}

void Colider::calcTransformedVertices(glm::mat4& transform,std::vector<std::array<glm::mat4,128>> animationMatrix)
{
	for (int i = 0;i < transformedVerticesPos.size();i++)
	{
		glm::mat4 animation = glm::mat4(0.0f);
		for (int j = 0; j < 4; j++)
		{
			animation += animationMatrix[meshColiderVertexOptions[i].skinIndex][j] * meshColiderVertexOptions[i].weight[j];
		}

		if (animation == glm::mat4(0.0f))
		{
			animation = glm::mat4(1.0f);
		}

		transformedVerticesPos[i] = transform * animation * glm::vec4(initialVertcesPos[i], 1.0f);
	}
}

bool Colider::meshIntersect(std::shared_ptr<Colider> oppColider, glm::vec3& collisionVec)
{
	//自身のインデックス配列、頂点配列を取得
	std::vector<uint32_t> indices = getMeshColiderIndices();
	std::vector<glm::vec3> vertices = getMeshColiderVertices();

	//相手のインデックス配列、頂点配列を取得
	std::vector<uint32_t> oppIndices = oppColider->getMeshColiderIndices();
	std::vector<glm::vec3> oppVertices = oppColider->getMeshColiderVertices();

	//衝突していたメッシュの数を数える
	int intersectCount = 0;

	//三角形同士の衝突判定を行う
	//衝突判定内では、すべての座標をp1を原点とする
	for (int i = 0; i < indices.size(); i += 3)
	{
		//自身のコライダーの三角形の頂点と面法線とその線分のベクトルを計算する
		glm::vec3 p1 = vertices[indices[i]];
		glm::vec3 p2 = vertices[indices[i + 1]] - p1;
		glm::vec3 p3 = vertices[indices[i + 2]] - p1;

		glm::vec3 meshNormal = glm::cross(p2, p3);
		std::array<glm::vec3, 3> points = { p1,p2,p3 };
		std::array<glm::vec3, 3> startPoints = { p1,p1,p2 };
		std::array<glm::vec3, 3> lines = { p2, p3,p3 - p2 };

		for (int j = 0; j < oppIndices.size(); j += 3)
		{
			//相手の三角形の頂点とその線分に加え、線分の始点も計算する
			glm::vec3 q1 = oppVertices[oppIndices[j]] - p1;
			glm::vec3 q2 = oppVertices[oppIndices[j + 1]] - p1;
			glm::vec3 q3 = oppVertices[oppIndices[j + 2]] - p1;

			std::array<glm::vec3, 3> oppLines = { q2,q3,q3 - q2 };
			std::array<glm::vec3, 3> oppStartPoints = { q1,q1,q2 };

			//p1を原点とする
			points[0] = glm::vec3(0.0f);

			//三角形との交点と線分押し点からの距離
			float distance;
			glm::vec3 intersectPoint;
			if (triangleIntersect(points,startPoints,lines, meshNormal, oppStartPoints, oppLines, distance, intersectPoint))
			{
				collisionVec += glm::normalize(meshNormal) * distance;
				intersectCount++;
			}
		}
	}

	if (intersectCount > 0)
	{
		//最後にすべての衝突解消ベクトルの平均をとる
		collisionVec /= intersectCount;
		std::cout << collisionVec << std::endl;

		return true;
	}

	return false;
}

//三角形同士の衝突判定と面の法線から解消ベクトルを計算する
bool Colider::triangleIntersect(std::array<glm::vec3, 3>& vertices,std::array<glm::vec3,3>& startPoints,std::array<glm::vec3,3>& lines,glm::vec3 meshNormal
	, std::array<glm::vec3, 3>& oppStartPoints, std::array<glm::vec3, 3>& oppLines,float& distance, glm::vec3& intersectPoint)
{
	for (int i = 0; i < 3; i++)
	{
		float t = 0.0f;

		int linePlaneIntersectCase = linePlaneIntersect(vertices[0], meshNormal, oppStartPoints[i], oppLines[i],t);

		if (linePlaneIntersectCase != -1)
		{
			std::cout << linePlaneIntersectCase << std::endl;
		}

		switch (linePlaneIntersectCase)
		{
		case -1:
			continue;
		case 0:
			//線分は平面上にはある
			//次は線分が三角形内にあるかしらべる
			for (int j = 0; j < 3; j++)
			{
				if (lintLineIntersect(startPoints[j],lines[j], oppStartPoints[i], oppLines[i], distance, intersectPoint))
				{
					return true;
				}
			}
			return false;

		case 1:
			//線分と平面は交差している
			//交差点は三角形上にあるかどうかを調べる

			//線分と交点までの距離と交点の座標
			if (lineTriangleIntersect(vertices, oppStartPoints[i], oppLines[i], distance, intersectPoint))
			{
				return true;
			}
			return false;
		}
	}

	return false;
}

//線分と無限平面上の関係をしらべる 
int Colider::linePlaneIntersect(glm::vec3 vertex, glm::vec3 meshNormal, glm::vec3 startPoint, glm::vec3 line, float& t)
{
	glm::vec3 point = vertex - startPoint;

	float t1 = glm::dot(meshNormal, point);
	float t2 = glm::dot(meshNormal, startPoint);

	if (t2 == 0.0f)
	{
		if (t1 == 0.0f)
		{//線分が平面上にある
			return 0;
		}
		else
		{//線分は平面と平行だが、線分が平面上にない
			return -1;
		}
	}
	else
	{
		if (t1 >= 0.0f && t1 <= 1.0f)
		{//線分と平面は交差している
			return 1;
		}
		else
		{//線分と平面は交差していない
			return -1;
		}
	}
}

//三角形と線分の衝突判定
bool Colider::lineTriangleIntersect(std::array<glm::vec3, 3>& vertices
	, glm::vec3 oppStartPoint, glm::vec3 oppLine, float& distance, glm::vec3& intersectPoint)
{
	//線分の逆ベクトルを得る
	glm::vec3 invLine = -oppLine;

	glm::mat3 mat;
	mat[0] = vertices[1];
	mat[1] = vertices[2];
	mat[2] = invLine;

	//クラメルの公式の分母
	float det1 = glm::determinant(mat);

	if (det1 <= 0.0f)
	{//線分と三角形が平行
		return false;
	}

	glm::vec3 vec = oppStartPoint - vertices[0];

	//式の各係数が0以上1以下かを調べる
	//条件を満たすとき、交点の座標は三角形上にある
	mat[0] = vec;
	mat[1] = vertices[2];
	mat[2] = invLine;
	float u = glm::determinant(mat) / det1;

	if (u >= 0.0f && u <= 1.0f)
	{
		//列優先に気を付ける
		mat[0] = vertices[1];
		mat[1] = vec;
		mat[2] = invLine;

		float v = glm::determinant(mat) / det1;
		if (v >= 0.0f && v <= 1.0f && (u + v) <= 1.0f)
		{
			mat[0] = vertices[1];
			mat[1] = vertices[2];
			mat[2] = vec;

			float t = glm::determinant(mat) / det1;

			if (t < 0.0f)
			{
				return false;
			}

			distance = t;
			intersectPoint = oppStartPoint + glm::normalize(oppLine) * t;

			return true;
		}
	}

	return false;
}

//線分同士の交差判定
bool Colider::lintLineIntersect(glm::vec3 startPoint, glm::vec3 endPoint, glm::vec3 startPoint2, glm::vec3 endPoint2
	, float& distance, glm::vec3& intersectPoint)
{
	glm::vec3 d1 = endPoint - startPoint;
	glm::vec3 d2 = endPoint2 - startPoint2;
	glm::vec3 d3 = startPoint - startPoint2;

	glm::vec3 cross = glm::cross(d1, d2);

	float dot = glm::dot(cross, cross);

	if (dot == 0.0f)
	{//線分は平行
		return false;
	}

	float u = glm::dot(glm::cross(d3, d2), cross) / dot;
	float v = glm::dot(glm::cross(d3, d1), cross) / dot;

	if (u >= 0.0f && u <= 1.0f
		&& v >= 0.0f && v <= 1.0f)
	{
		distance = u;
		intersectPoint = startPoint + u * d1;

		return true;
	}

	return false;
}