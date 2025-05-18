#pragma once

#include<map>
#include<array>

#include"Object.h"
#include"GltfModel.h"
#include"EnumList.h"

#include<random>

//GJK�̍\����
struct Simplex
{
private:
	//GJK�@�ň����ő�4�̒��_�����}�`
	std::array<glm::vec3, 4> points;

public:
	//�L���Ȓ��_�̐�
	int size;

	Simplex()
	{
		size = 0;
		glm::vec3 zero = glm::vec3(0.0f, 0.0f, 0.0f);
		points = { zero };
	}

	//���_�̒ǉ�
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

	//epa�@�ł̍\���ւ̃R�s�[
	void setSimplexVertices(std::vector<glm::vec3>& polytope)
	{
		polytope.resize(points.size());
		std::copy(points.begin(), points.end(), polytope.begin());
	}
};

//�����蔻��p�̃N���X
class Colider
{
private:

	//�R���C�_�[�������蔻����������邩�ǂ���
	bool trigger;

	//AABB��̍���A�E���̒��_
	glm::vec3 min, max;
	//AABB�ɍ��W�ϊ�������������
	glm::vec3 transformedMin, transformedMax;

	//�g��s��
	glm::mat4 scaleMat;
	
	//���݂̃R���C�_�[��AABB�̒��_
	std::vector<glm::vec3> coliderVertices;
	//���W�ϊ���������O�̃R���C�_�[�̒��_�̍��W
	std::vector<glm::vec3> originalVertexPos;

	//�R���C�_�[�����蔻��p�p�̃C���f�b�N�X
	std::vector<uint32_t> coliderIndices;
	//�`��p�̃C���f�b�N�X
	std::vector<uint32_t> drawColiderIndices;
	//SAT���p���̃C���f�b�N�X
	std::vector<uint32_t> satIndices;

	//�R���C�_�[�`��p�̕ϐ�
	BufferObject pointBuffer;
	MappedBuffer mappedBuffer;
	DescriptorInfo descInfo;
	DescSetData descSetData;

	//�����̕����x�N�g���̂ق��ɂ��̒��_�����邩�ǂ���
	bool sameDirection(glm::vec3 dir, glm::vec3 point) { return glm::dot(dir, point) > 0.0f; }

	//�������m�ł킩��͈͂Ń~���R�t�X�L�[�������_���܂݂��������ׂ�
	bool Line(Simplex& simplex,glm::vec3& dir);
	//�O�p�`�ł킩��͈͂Ń~���R�t�X�L�[�������_���܂݂��������ׂ�
	bool Triangle(Simplex& simplex, glm::vec3& dir);
	//�l�p���ł킩��͈͂Ń~���R�t�X�L�[�������_���܂ނ����ׂ�
	bool Tetrahedron(Simplex& simplex, glm::vec3& dir);

	//�����̕����x�N�g���̌����ōł��������_�����߂�
	glm::vec3 getFurthestPoint(glm::vec3 dir);
	//�T�|�[�g�ʑ������߂�
	glm::vec3 getSupportVector(std::shared_ptr<Colider> oppColider, glm::vec3 dir);
	//GJK�̎��̒P�̂����߂�
	bool nextSimplex(Simplex& simplex, glm::vec3& dir);
	//���̐�������ł��߂����_�����߂�
	glm::vec3 getClosestLineToVertex(glm::vec3 lineStart, glm::vec3 lineFinish, glm::vec3 point);
	//GJK�@�ł̓����蔻������s
	bool GJK(std::shared_ptr<Colider> oppColider,glm::vec3& collisionDepthVec);
	//GJK�@���EPA�@�ŏՓ˂��������邽�߂̃x�N�g�����擾
	void EPA(std::shared_ptr<Colider> oppColider, Simplex& simplex, glm::vec3& collisionDepthVec);
	//�������藝�𗘗p���������蔻������s�A�Փ˂��������邽�߂̃x�N�g�����v�Z
	bool SAT(std::shared_ptr<Colider> oppColider, float& collisionDepth, glm::vec3& collisionNormal);
	//����̐������܂܂Ȃ���΂��̒��_��P�̂Ɋ܂߂�
	void addIfUniqueEdge(std::vector<std::pair<size_t, size_t>>& edges, const std::vector<size_t>& faces,
						 size_t a, size_t b);
	//�ʂ̖@�����擾
	std::pair<std::vector<glm::vec4>, size_t> getFaceNormals(
		std::vector<glm::vec3>& vertices,
		std::vector<size_t>& faces);

	//gltf���f�����璸�_���擾����
	void setVertices(GltfNode* node, int& loadedVertexCount, int& loadedIndexCount);

public:

	Colider(std::shared_ptr<GltfModel> model,bool isTrigger);
	~Colider();

	bool isTrigger()
	{
		return trigger;
	}

	//Model�N���X�̏������W������W�ϊ���K�p����
	void initFrameSettings(glm::vec3 initScale);
	//�R���C�_�[�̃X�P�[����ݒ�
	glm::vec3 scale;
	//�R���C�_�[�̃X�P�[���s����擾
	glm::mat4 getScaleMat();
	//Model�N���X�̈ړ��Ȃǂ��R���C�_�[�ɂ����f
	void reflectMovement(glm::mat4& transform);

	//�T�|�[�g�ʑ������߂�(SAT�p)
	void projection(float& min, float& max, glm::vec3& minVertex, glm::vec3& maxVertex, glm::vec3& axis);
	//descriptorSet�̐ݒ�
	void setDescriptorSet(VkDescriptorSet descriptorSet);
	
	//�R���C�_�[�̃����_�����O�p�̕ϐ�
	BufferObject* getPointBufferData();
	MappedBuffer* getMappedBufferData();
	DescriptorInfo& getDescInfo();
	DescSetData& getDescSetData() { return descSetData; }

	//SAT�p�����蔻��̎��s
	virtual bool Intersect(std::shared_ptr<Colider> oppColider, glm::vec3& collisionVector);
	//GJK�p�����蔻��̎��s
	virtual bool Intersect(std::shared_ptr<Colider> oppColider);
	//�{�b�N�X���C�L���X�g�p�̓����蔻��̎��s
	virtual bool Intersect(glm::vec3 origin, glm::vec3 dir, float length,glm::vec3& normal);

	//���W�ϊ��������Ă��Ȃ��R���C�_�[�̒��_���擾
	glm::vec3* getColiderOriginalVertices();
	//���W�ϊ����������R���C�_�[�̒��_���擾
	glm::vec3* getColiderVertices();
	//�R���C�_�[�̒��_���擾
	int getColiderVerticesSize();
	//�R���C�_�[�̒��_�̃C���f�b�N�X���擾
	int* getColiderIndices();
	//�R���C�_�[�̒��_�̃C���f�b�N�X�̃T�C�Y���擾
	int getColiderIndicesSize();
	//�`��p�̃C���f�b�N�X�̃T�C�Y���擾
	int getDrawColiderIndicesSize() { return static_cast<int>(drawColiderIndices.size()); }
	//�`��p�̃C���f�b�N�X���擾
	uint32_t* getDrawColiderIndices() { return drawColiderIndices.data(); }
};