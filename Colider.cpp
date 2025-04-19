#include"Colider.h"

#include"VulkanBase.h"

Colider::Colider(glm::vec3 min, glm::vec3 max)
{
	isConvex = false;

	this->min = min;
	this->max = max;

	this->transformedMin = min;
	this->transformedMax = max;

	scale = glm::vec3(1.0f);
	scaleMat = glm::scale(scale);

	originalVertexPos.resize(8);//���W�ϊ��O�̍��W�z��

	originalVertexPos[0] = glm::vec3(min.x, min.y, min.z);
	originalVertexPos[1] = glm::vec3(max.x, min.y, min.z);
	originalVertexPos[2] = glm::vec3(max.x, min.y, max.z);
	originalVertexPos[3] = glm::vec3(min.x, min.y, max.z);
	originalVertexPos[4] = glm::vec3(min.x, max.y, min.z);
	originalVertexPos[5] = glm::vec3(max.x, max.y, min.z);
	originalVertexPos[6] = glm::vec3(max.x, max.y, max.z);
	originalVertexPos[7] = glm::vec3(min.x, max.y, max.z);

	coliderIndices =
	{
		//����
		0,1,3,
		1,2,3,
		//���
		4,7,5,
		7,6,5,
		//�O��
		0,4,1,
		4,5,1,
		//�w��
		3,2,7,
		2,6,7,
		//����
		0,3,4,
		3,7,4,
		//�E��
		1,5,2,
		5,6,2
	};

	satIndices.resize(3 * 6);
	satIndices = { 0,4,5,1,5,6,6,2,3,3,7,4,2,1,0,6,5,4 };
	satIndices = { 1,0,2,4,5,6,5,6,1,4,0,7,5,4,1,7,6,3 };//�Փ˔���p�̃C���f�b�N�X�z��

	descSetData.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
}

Colider::Colider(std::shared_ptr<GltfModel> model)
{
	isConvex = true;

	scale = glm::vec3(1.0f);
	scaleMat = glm::scale(scale);

	originalVertexPos.resize(model->vertexNum);
	convexVertexData.resize(model->vertexNum);
	coliderIndices.resize(model->indexNum);

	int loadedVertexCount = 0;
	int loadedIndexCount = 0;

	setVertices(model->getRootNode(), loadedVertexCount, loadedIndexCount);

	descSetData.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
}

//�ʖ@�̃I�u�W�F�N�g�悤�ɒ��_��ݒ肷��
void Colider::setVertices(GltfNode* node,int& loadedVertexCount,int& loadedIndexCount)
{
	for (int i = 0; i < node->children.size(); i++)
	{
		setVertices(node->children[i], loadedVertexCount, loadedIndexCount);
	}

	for(auto mesh :node->meshArray)
	{
		if (mesh)
		{
			glm::mat4 nodeMat = node->getMatrix();

			for (int i = 0; i < mesh->vertices.size(); i++)
			{
				originalVertexPos[i + loadedVertexCount] = mesh->vertices[i].pos;

				convexVertexData[i + loadedVertexCount].skinIndex = node->globalHasSkinNodeIndex;
				convexVertexData[i + loadedVertexCount].boneID = mesh->vertices[i].boneID1;
				convexVertexData[i + loadedVertexCount].weight = mesh->vertices[i].weight1;
				convexVertexData[i + loadedVertexCount].nodeMatrix = nodeMat;
			}

			for (int i = 0; i < mesh->indices.size(); i++)
			{
				coliderIndices[i + loadedIndexCount] = mesh->indices[i] + loadedVertexCount;
			}

			loadedVertexCount += static_cast<int>(mesh->vertices.size());
			loadedIndexCount += static_cast<int>(mesh->indices.size());
		}
	}
}

//Model�N���X�̏������W������W�ϊ���K�p����
void Colider::initFrameSettings()
{
	scaleMat = glm::scale(scale);

	for (int i = 0; i < originalVertexPos.size(); i++)
	{
		originalVertexPos[i] = scaleMat * glm::vec4(originalVertexPos[i], 1.0);
	}

	coliderVertices.resize(originalVertexPos.size());
	std::copy(originalVertexPos.begin(), originalVertexPos.end(), coliderVertices.begin());
}

//���W�ϊ����������R���C�_�[�̒��_���擾
glm::vec3* Colider::getColiderVertices()
{
	return coliderVertices.data();
}

//�R���C�_�[�̒��_���擾
glm::vec3* Colider::getColiderOriginalVertices()
{
	return originalVertexPos.data();
}

//�R���C�_�[�̒��_���擾
int Colider::getColiderVerticesSize()
{
	return static_cast<int>(originalVertexPos.size());
}

//�R���C�_�[�̒��_�̃C���f�b�N�X���擾
int* Colider::getColiderIndices()
{
	return (int*)coliderIndices.data();
}

//�R���C�_�[�̒��_�̃C���f�b�N�X�̃T�C�Y���擾
int Colider::getColiderIndicesSize()
{
	return static_cast<int>(coliderIndices.size());
}

//�R���C�_�[�̃X�P�[���s����擾
glm::mat4 Colider::getScaleMat()
{
	return scaleMat;
}

//Model�N���X�̈ړ��Ȃǂ��R���C�_�[�ɂ����f
void Colider::reflectMovement(glm::mat4& transform)
{
	for (int i = 0; i < coliderVertices.size(); i++)
	{
		coliderVertices[i] = transform * glm::vec4(originalVertexPos[i], 1.0f);
		transformedMin = transform * glm::vec4(min, 1.0f);
		transformedMax = transform * glm::vec4(max, 1.0f);
	}
}

//Model�N���X�̈ړ��Ȃǂ��R���C�_�[�ɂ����f
void Colider::reflectMovement(glm::mat4& transform,std::vector<std::array<glm::mat4, 128>>& jointMatrices)
{
	glm::mat4 skinMat;

	for (int i = 0; i < coliderVertices.size(); i++)
	{
		skinMat = glm::mat4(0.0f);
		for (int j = 0; j < 4; j++)
		{
			skinMat += jointMatrices[convexVertexData[i].skinIndex][convexVertexData[i].boneID[j]] * convexVertexData[i].weight[j];
		}
		if (skinMat == glm::mat4(0.0f) || convexVertexData[i].skinIndex == -1)
		{
			skinMat = glm::mat4(1.0f);
		}

		coliderVertices[i] = transform * convexVertexData[i].nodeMatrix * skinMat * glm::vec4(originalVertexPos[i], 1.0f);
		transformedMin = transform * glm::vec4(min, 1.0f);
		transformedMax = transform * glm::vec4(max, 1.0f);
	}
}

//SAT�p�����蔻��̎��s
bool Colider::Intersect(std::shared_ptr<Colider> oppColider, glm::vec3& collisionVector)
{
	float collisionDepth;
	bool collision = false;
	
	collision = GJK(oppColider,collisionVector);

	/*
	collision = SAT(oppColider, collisionDepth, collisionVector);
	collisionVector = collisionVector * collisionDepth;
	*/

	return collision;
}

//GJK�p�����蔻��̎��s
bool Colider::Intersect(std::shared_ptr<Colider> oppColider)
{
	float collisionDepth;
	bool collision = false;

	glm::vec3 collisionVector;

	collision = GJK(oppColider, collisionVector);

	/*
	collision = SAT(oppColider, collisionDepth, collisionVector);
	collisionVector = collisionVector * collisionDepth;
	*/

	return collision;
}

//�{�b�N�X���C�L���X�g�p�̓����蔻��̎��s
bool Colider::Intersect(glm::vec3 origin, glm::vec3 dir, float length,glm::vec3& normal)
{
	/*
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
	*/

	const float epsilon = 0.0001f;

	for (int i = 0; i < coliderIndices.size(); i += 3)
	{
		glm::vec3 e1 = coliderVertices[coliderIndices[i + 1]] - coliderVertices[coliderIndices[i]];
		glm::vec3 e2 = coliderVertices[coliderIndices[i + 2]] - coliderVertices[coliderIndices[i]];

		glm::vec3 h = glm::cross(dir, e2);

		float dot = glm::dot(h, e1);
		if (dot >= -epsilon && dot <= epsilon)
		{//�����͎O�p�`�ƕ��s
			continue;
		}

		float inv = 1.0f / dot;
		glm::vec3 v = origin - coliderVertices[coliderIndices[i]];
		float u = inv * glm::dot(h, v);

		if (u < 0.0f || u > 1.0f)
		{
			continue;
		}

		glm::vec3 q = glm::cross(v, e1);
		float f = inv * glm::dot(dir, q);
		if (f < 0.0f || u + f > 1.0f)
		{
			continue;
		}

		float t = inv * glm::dot(e2, q);

		if (t > -epsilon && t <= length)
		{
			normal = glm::normalize(glm::cross(e1, e2));

			return true;
		}
	}

	return false;
}

//�������藝�𗘗p���������蔻������s�A�Փ˂��������邽�߂̃x�N�g�����v�Z
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

//�T�|�[�g�ʑ������߂�(SAT�p)
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

//GJK�@�ł̓����蔻������s
bool Colider::GJK(std::shared_ptr<Colider> oppColider,glm::vec3& collisionDepthVec)
{
	glm::vec3 support = getSupportVector(oppColider, glm::vec3(1.0f, 0.0f, 0.0f));//�K���ȕ����̃T�|�[�g�ʑ������߂�

	Simplex simplex;
	simplex.push_front(support);

	glm::vec3 dir = -support;

	int count = 50;

	while (count > 0)//�����񐔂ɐ�����݂���
	{
		support = getSupportVector(oppColider, dir);//�T�|�[�g�ʑ������߂�

		if (glm::dot(support, dir) <= 0.0f)//�������߂��T�|�[�g�ʑ������_�̕����ƌ������t��������A�����蔻����I��
		{
			return false;
		}

		simplex.push_front(support);//�x�N�g����ǉ�

		if (nextSimplex(simplex, dir))//�P�̂̍X�V�A�l�p�����Ɍ��_���܂܂�Ă�����AEPA�Ɉڍs
		{
			EPA(oppColider,simplex, collisionDepthVec);//�Փ˂��������邽�߂̃x�N�g���̌v�Z
			return true;
		}

		count--;
	}

	return false;
}

//�����̕����x�N�g���̌����ōł��������_�����߂�
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

//�T�|�[�g�ʑ������߂�
glm::vec3 Colider::getSupportVector(std::shared_ptr<Colider> oppColider, glm::vec3 dir)
{
	return getFurthestPoint(dir) - oppColider->getFurthestPoint(-dir);
}

//�������m�ł킩��͈͂Ń~���R�t�X�L�[�������_���܂݂��������ׂ�
bool Colider::Line(Simplex& simplex, glm::vec3& dir)
{
	glm::vec3 a = simplex[0];
	glm::vec3 b = simplex[1];

	glm::vec3 ab = b - a;
	glm::vec3 ao = -a;//a0:���_�����̃x�N�g��

	if (sameDirection(ab, ao))//b�����_�����Ɉʒu���Ȃ�=�P�̂��O�p�`�ɂ��Ă����_���܂܂Ȃ�
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

//�O�p�`�ł킩��͈͂Ń~���R�t�X�L�[�������_���܂݂��������ׂ�
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

//�l�p���ł킩��͈͂Ń~���R�t�X�L�[�������_���܂ނ����ׂ�
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

//GJK�̎��̒P�̂����߂�
bool Colider::nextSimplex(Simplex& simplex, glm::vec3& dir)
{
	switch (simplex.size) {
	case 2: return Line(simplex, dir);
	case 3: return Triangle(simplex, dir);
	case 4: return Tetrahedron(simplex, dir);
	}

	return false;
}

//GJK�@���EPA�@�ŏՓ˂��������邽�߂̃x�N�g�����擾
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

	collisionDepthVec = minNormal * (minDistance + 0.0000f);//�V�т��Ƃ��Ă���
}

//����̐������܂܂Ȃ���΂��̒��_��P�̂Ɋ܂߂�
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

//�ʂ̖@�����擾
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

//�R���C�_�[�p��gpu��̃o�b�t�@�̔j��
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