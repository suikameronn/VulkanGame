#pragma once

#include<map>
#include<array>

#include"Object.h"
#include"GltfModel.h"

#include"GpuBufferFactory.h"
#include"DescriptorSetFactory.h"

#include"MeshRendererComp.h"

#include"TransformComp.h"

#include<random>

struct SimplexVertex
{
	//�~���R�t�X�L�[��ԏ�̍��W
	glm::vec3 point;

	//��̍��W������o�����R���C�_�[�̒��_�̃y�A�̈��
	glm::vec3 myPoint;

	//����̃R���C�_�[�̃y�A�̈��
	glm::vec3 oppPoint;

	SimplexVertex()
	{
		point = glm::vec3(0.0f);

		myPoint = glm::vec3(0.0f);

		oppPoint = glm::vec3(0.0f);
	}
};

//GJK�̍\����
struct Simplex
{
private:
	//GJK�@�ň����ő�4�̒��_�����}�`
	std::array<SimplexVertex, 4> vertices;

public:
	//�L���Ȓ��_�̐�
	int size;

	Simplex()
	{
		size = 0;
		glm::vec3 zero = glm::vec3(0.0f, 0.0f, 0.0f);
		vertices = {};
	}

	//���_�̒ǉ�
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

	//epa�@�ł̍\���ւ̃R�s�[
	void setSimplexVertices(std::vector<SimplexVertex>& polytope)
	{
		polytope.resize(vertices.size());
		std::copy(vertices.begin(), vertices.end(), polytope.begin());
	}
};

//�����蔻��p�̃N���X
class Colider
{
private:

	//�R���C�_�[�������蔻����������邩�ǂ���
	bool trigger;

	//�R���C�_�[���g�̃I�t�Z�b�g
	glm::vec3 offsetPos;
	glm::vec3 offsetScale;

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

	//�R���C�_�[�`��p�̃t�@�N�g���[
	std::shared_ptr<GpuBufferFactory> bufferFactory;
	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;
	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;

	//�R���C�_�[�`��p�̕ϐ�
	std::shared_ptr<GpuBuffer> vertBuffer;
	std::shared_ptr<GpuBuffer> indeBuffer;
	std::shared_ptr<GpuBuffer> matBuffer;
	std::shared_ptr<DescriptorSet> descriptorSet;

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
	SimplexVertex getSupportVector(const std::unique_ptr<Colider>& oppColider, glm::vec3 dir);
	//GJK�̎��̒P�̂����߂�
	bool nextSimplex(Simplex& simplex, glm::vec3& dir);
	//���̐�������ł��߂����_�����߂�
	glm::vec3 getClosestLineToVertex(glm::vec3 lineStart, glm::vec3 lineFinish, glm::vec3 point);
	//GJK�@�ł̓����蔻������s
	bool GJK(const std::unique_ptr<Colider>& oppColider,glm::vec3& collisionDepthVec, glm::vec3& myCollisionPoint, glm::vec3& oppCollisionPoint);
	//GJK�@���EPA�@�ŏՓ˂��������邽�߂̃x�N�g�����擾
	void EPA(const std::unique_ptr<Colider>& oppColider, Simplex& simplex, glm::vec3& collisionDepthVec, glm::vec3& myCollisionPoint, glm::vec3& oppCollisionPoint);
	//�Փ˓_���v�Z����
	void CollisionPoint(const std::array<SimplexVertex, 3>& triangle, const glm::vec3& point, glm::vec3& myCollisionPoint, glm::vec3& oppCollisionPoint);
	//�O�p�`��̒��_�̏d�S���W�����߂�
	std::array<float, 3> CenterCoord(const std::array<glm::vec3, 3>& triangle, const glm::vec3& position);
	//�������藝�𗘗p���������蔻������s�A�Փ˂��������邽�߂̃x�N�g�����v�Z
	int intersectRectQuad2(float* h, float* p, float* ret);
	bool SAT(const std::unique_ptr<Colider>& oppColider, float& collisionDepth, glm::vec3& collisionNormal);
	bool SAT(const std::unique_ptr<Colider>& oppColider, glm::vec3& collisionNormal,glm::vec3& myCollisionPoint,glm::vec3& oppCollisionPoint
		,const TransformComp& myTransform,const TransformComp& oppTransform);
	//����̐������܂܂Ȃ���΂��̒��_��P�̂Ɋ܂߂�
	void addIfUniqueEdge(std::vector<std::pair<int, int>>& edges, const std::vector<int>& faces,
						 int a, int b);
	//�ʂ̖@�����擾
	std::pair<std::vector<glm::vec4>, int> getFaceNormals(
		std::vector<SimplexVertex>& vertices,
		std::vector<int>& faces);

	//gltf���f�����璸�_���擾����
	void setVertices(GltfNode* node, int& loadedVertexCount, int& loadedIndexCount);

public:

	Colider(std::shared_ptr<GltfModel> model,bool isTrigger
		, std::shared_ptr<GpuBufferFactory> buffer, std::shared_ptr<DescriptorSetLayoutFactory> layout
		, std::shared_ptr<DescriptorSetFactory> desc);

	bool isTrigger()
	{
		return trigger;
	}

	//�`��ɕK�v�ȃ��\�[�X���쐬����
	void createBuffer();
	void createDescriptorSet();

	//�R���C�_�[�̊e�I�t�Z�b�g��ݒ肷��
	void setOffsetPos(const glm::vec3& pos);
	void setOffsetScale(const glm::vec3& scale);

	//Model�N���X�̏������W������W�ϊ���K�p����
	void initFrameSettings(glm::vec3 initScale);
	//�R���C�_�[�̃X�P�[����ݒ�
	glm::vec3 scale;
	//�R���C�_�[�̃X�P�[���s����擾
	glm::mat4 getScaleMat();
	//Model�N���X�̈ړ��Ȃǂ��R���C�_�[�ɂ����f
	void reflectMovement(const glm::vec3& translate, const glm::mat4& rotate, const glm::vec3& scale);
	//�d�S��Ԃ�
	glm::vec3 getCenterPos();

	//�T�|�[�g�ʑ������߂�(SAT�p)
	void projection(float& min, float& max, glm::vec3& minVertex, glm::vec3& maxVertex, glm::vec3& axis);
	//descriptorSet�̐ݒ�
	void setDescriptorSet(VkDescriptorSet descriptorSet);
	
	//�R���C�_�[�̃����_�����O�p�̕ϐ�
	std::shared_ptr<GpuBuffer> getVertexBuffer() { return vertBuffer; }
	std::shared_ptr<GpuBuffer> getIndexBuffer() { return indeBuffer; }
	std::shared_ptr<DescriptorSet> getDescriptorSet() { return descriptorSet; }

	//SAT�p�����蔻��̎��s
	virtual bool Intersect(const std::unique_ptr<Colider>& oppColider, glm::vec3& collisionVector, glm::vec3& myCollisionPoint, glm::vec3& oppCollisionPoint
		, const TransformComp& myTransform, const TransformComp& oppTransform);
	//GJK�p�����蔻��̎��s
	virtual bool Intersect(const std::unique_ptr<Colider>& oppColider);
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