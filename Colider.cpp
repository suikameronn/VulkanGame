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

	originalVertexPos.resize(8);//���W�ϊ��O�̍��W�z��

	originalVertexPos[0] = glm::vec3(min.x, min.y, min.z);
	originalVertexPos[1] = glm::vec3(max.x, min.y, min.z);
	originalVertexPos[2] = glm::vec3(max.x, min.y, max.z);
	originalVertexPos[3] = glm::vec3(min.x, min.y, max.z);
	originalVertexPos[4] = glm::vec3(min.x, max.y, min.z);
	originalVertexPos[5] = glm::vec3(max.x, max.y, min.z);
	originalVertexPos[6] = glm::vec3(max.x, max.y, max.z);
	originalVertexPos[7] = glm::vec3(min.x, max.y, max.z);

	coliderIndices.resize(12);

	coliderIndices = { 1,0,1,2,2,3,3,0,0,4,1,5,2,6,3,7,4,5,5,6,6,7,7,4 };//�`��p�̃C���f�b�N�X�z��

	satIndices.resize(3 * 6);
	satIndices = { 0,4,5,1,5,6,6,2,3,3,7,4,2,1,0,6,5,4 };
	satIndices = { 1,0,2,4,5,6,5,6,1,4,0,7,5,4,1,7,6,3 };//�Փ˔���p�̃C���f�b�N�X�z��

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

//Model�N���X�̏������W������W�ϊ���K�p����
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

	if (isMeshColider)
	{
		calcTransformedVertices(transform);
	}
}

//Model�N���X�̈ړ��Ȃǂ��R���C�_�[�ɂ����f
void Colider::reflectMovement(glm::mat4& transform,std::vector<std::array<glm::mat4,128>>& animationMatrix)
{
	//AABB�̍X�V
	for (int i = 0; i < coliderVertices.size(); i++)
	{
		coliderVertices[i] = transform * glm::vec4(originalVertexPos[i], 1.0f);
		transformedMin = transform * glm::vec4(min, 1.0f);
		transformedMax = transform * glm::vec4(max, 1.0f);
	}

	if (isMeshColider)
	{//���b�V���R���C�_�[�̏ꍇ�A���b�V���̍X�V
		calcTransformedVertices(transform,animationMatrix);
	}
}

//SAT�p�����蔻��̎��s
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

//GJK�p�����蔻��̎��s
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

//�{�b�N�X���C�L���X�g�p�̓����蔻��̎��s
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

	collisionDepthVec = minNormal * (minDistance + 0.001f);//�V�т��Ƃ��Ă���
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

/*���b�V���R���C�_�[�p�̊֐�*/////////////////////////////////////////////////////////////////////////////////////

//���b�V���R���C�_�[�̃C���f�b�N�X���擾�A���b�V���ł͂Ȃ��ꍇ�́A�R���C�_�[�`��p�̃C���f�b�N�X��n��
std::vector<uint32_t>& Colider::getMeshColiderIndices()
{
	if (isMeshColider)
	{
		return meshColiderIndices;
	}

	//���b�V���łȂ��R���C�_�[�̏ꍇ
	//�R���C�_�[�̕`��p�̃C���f�b�N�X��n��
	return coliderIndices;
}

//���b�V���R���C�_�[�̓����蔻�莞�Ɏ��g�̃R���C�_�[�̃^�C�v�ɍ��킹�����_�z���n��
std::vector<glm::vec3>& Colider::getMeshColiderVertices()
{
	if (isMeshColider)
	{
		return transformedVerticesPos;
	}

	return coliderVertices;
}

//gltfModel�̏����̍��W�̒��_���R���C�_�[�ɃR�s�[����
void Colider::setMeshColider(std::shared_ptr<GltfModel> gltfModel)
{
	if (!isMeshColider)
	{
		return;
	}

	gltfModel->setModelVertexIndex(initialVertcesPos,meshColiderVertexOptions, meshColiderIndices);

	std::copy(initialVertcesPos.begin(), initialVertcesPos.end(), transformedVerticesPos.begin());
}

//�R���C�_�[�̒��_�ɍ��W�ϊ���������(�A�j���[�V�����͂Ȃ�)
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
	//���g�̃C���f�b�N�X�z��A���_�z����擾
	std::vector<uint32_t> indices = getMeshColiderIndices();
	std::vector<glm::vec3> vertices = getMeshColiderVertices();

	//����̃C���f�b�N�X�z��A���_�z����擾
	std::vector<uint32_t> oppIndices = oppColider->getMeshColiderIndices();
	std::vector<glm::vec3> oppVertices = oppColider->getMeshColiderVertices();

	//�Փ˂��Ă������b�V���̐��𐔂���
	int intersectCount = 0;

	//�O�p�`���m�̏Փ˔�����s��
	//�Փ˔�����ł́A���ׂĂ̍��W��p1�����_�Ƃ���
	for (int i = 0; i < indices.size(); i += 3)
	{
		//���g�̃R���C�_�[�̎O�p�`�̒��_�Ɩʖ@���Ƃ��̐����̃x�N�g�����v�Z����
		glm::vec3 p1 = vertices[indices[i]];
		glm::vec3 p2 = vertices[indices[i + 1]] - p1;
		glm::vec3 p3 = vertices[indices[i + 2]] - p1;

		glm::vec3 meshNormal = glm::cross(p2, p3);
		std::array<glm::vec3, 3> points = { p1,p2,p3 };
		std::array<glm::vec3, 3> startPoints = { p1,p1,p2 };
		std::array<glm::vec3, 3> lines = { p2, p3,p3 - p2 };

		for (int j = 0; j < oppIndices.size(); j += 3)
		{
			//����̎O�p�`�̒��_�Ƃ��̐����ɉ����A�����̎n�_���v�Z����
			glm::vec3 q1 = oppVertices[oppIndices[j]] - p1;
			glm::vec3 q2 = oppVertices[oppIndices[j + 1]] - p1;
			glm::vec3 q3 = oppVertices[oppIndices[j + 2]] - p1;

			std::array<glm::vec3, 3> oppLines = { q2,q3,q3 - q2 };
			std::array<glm::vec3, 3> oppStartPoints = { q1,q1,q2 };

			//p1�����_�Ƃ���
			points[0] = glm::vec3(0.0f);

			//�O�p�`�Ƃ̌�_�Ɛ��������_����̋���
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
		//�Ō�ɂ��ׂĂ̏Փˉ����x�N�g���̕��ς��Ƃ�
		collisionVec /= intersectCount;
		std::cout << collisionVec << std::endl;

		return true;
	}

	return false;
}

//�O�p�`���m�̏Փ˔���Ɩʂ̖@����������x�N�g�����v�Z����
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
			//�����͕��ʏ�ɂ͂���
			//���͐������O�p�`���ɂ��邩����ׂ�
			for (int j = 0; j < 3; j++)
			{
				if (lintLineIntersect(startPoints[j],lines[j], oppStartPoints[i], oppLines[i], distance, intersectPoint))
				{
					return true;
				}
			}
			return false;

		case 1:
			//�����ƕ��ʂ͌������Ă���
			//�����_�͎O�p�`��ɂ��邩�ǂ����𒲂ׂ�

			//�����ƌ�_�܂ł̋����ƌ�_�̍��W
			if (lineTriangleIntersect(vertices, oppStartPoints[i], oppLines[i], distance, intersectPoint))
			{
				return true;
			}
			return false;
		}
	}

	return false;
}

//�����Ɩ������ʏ�̊֌W������ׂ� 
int Colider::linePlaneIntersect(glm::vec3 vertex, glm::vec3 meshNormal, glm::vec3 startPoint, glm::vec3 line, float& t)
{
	glm::vec3 point = vertex - startPoint;

	float t1 = glm::dot(meshNormal, point);
	float t2 = glm::dot(meshNormal, startPoint);

	if (t2 == 0.0f)
	{
		if (t1 == 0.0f)
		{//���������ʏ�ɂ���
			return 0;
		}
		else
		{//�����͕��ʂƕ��s�����A���������ʏ�ɂȂ�
			return -1;
		}
	}
	else
	{
		if (t1 >= 0.0f && t1 <= 1.0f)
		{//�����ƕ��ʂ͌������Ă���
			return 1;
		}
		else
		{//�����ƕ��ʂ͌������Ă��Ȃ�
			return -1;
		}
	}
}

//�O�p�`�Ɛ����̏Փ˔���
bool Colider::lineTriangleIntersect(std::array<glm::vec3, 3>& vertices
	, glm::vec3 oppStartPoint, glm::vec3 oppLine, float& distance, glm::vec3& intersectPoint)
{
	//�����̋t�x�N�g���𓾂�
	glm::vec3 invLine = -oppLine;

	glm::mat3 mat;
	mat[0] = vertices[1];
	mat[1] = vertices[2];
	mat[2] = invLine;

	//�N�������̌����̕���
	float det1 = glm::determinant(mat);

	if (det1 <= 0.0f)
	{//�����ƎO�p�`�����s
		return false;
	}

	glm::vec3 vec = oppStartPoint - vertices[0];

	//���̊e�W����0�ȏ�1�ȉ����𒲂ׂ�
	//�����𖞂����Ƃ��A��_�̍��W�͎O�p�`��ɂ���
	mat[0] = vec;
	mat[1] = vertices[2];
	mat[2] = invLine;
	float u = glm::determinant(mat) / det1;

	if (u >= 0.0f && u <= 1.0f)
	{
		//��D��ɋC��t����
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

//�������m�̌�������
bool Colider::lintLineIntersect(glm::vec3 startPoint, glm::vec3 endPoint, glm::vec3 startPoint2, glm::vec3 endPoint2
	, float& distance, glm::vec3& intersectPoint)
{
	glm::vec3 d1 = endPoint - startPoint;
	glm::vec3 d2 = endPoint2 - startPoint2;
	glm::vec3 d3 = startPoint - startPoint2;

	glm::vec3 cross = glm::cross(d1, d2);

	float dot = glm::dot(cross, cross);

	if (dot == 0.0f)
	{//�����͕��s
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