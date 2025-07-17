#pragma once

#include"IComponent.h"

#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include"GpuBufferFactory.h"
#include"DescriptorSetFactory.h"

struct Transform
{
	alignas(16) glm::vec3 uvScale;
	glm::mat4 modelMatrix;
};

class TransformComponent : public IComponent
{
private:

	Transform transform;

	//GPU�̃o�b�t�@���쐬����
	std::shared_ptr<GpuBufferFactory> bufferFactory;

	//�f�B�X�N���v�^�Z�b�g�̃��C�A�E�g���쐬����
	std::shared_ptr<GpuDescriptorSetLayoutFactory> layoutFactory;

	//�o�b�t�@���Q�Ƃ��邽�߂̃f�B�X�N���v�^���쐬����
	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;

	//�I�u�W�F�N�g�̒��S
	glm::vec3 pivot;

	//���f���s��ƃr���[�s����|�����킹���s����L�^�����o�b�t�@
	std::shared_ptr<GpuBuffer> matBuffer;

	//�s��̋L�^���ꂽ�o�b�t�@�����ѕt����
	std::shared_ptr<DescriptorSet> matBufferBindDesc;

	//�s����L�^���郆�j�t�H�[���o�b�t�@���쐬����
	void createUniformBuffer();

	//���j�t�H�[���o�b�t�@�������f�B�X�N���v�^�Z�b�g���쐬����
	void createDescriptorSet();

public:

	glm::vec3 position;
	glm::vec3 rotateDeg;
	glm::vec3 scale;

	glm::vec3 getPivot() { return pivot; }

	TransformComponent(std::shared_ptr<GpuBufferFactory> buffer
		,std::shared_ptr<DescriptorSetFactory> desc
		, std::shared_ptr<GpuDescriptorSetLayoutFactory> layout);

	//���s�ړ��s��
	glm::mat4 tlanslateMat();

	//��]�s��
	glm::mat4 rotateMat();

	//�g��s��
	glm::mat4 scaleMat();

	//TRS�s��
	glm::mat4 transformMat();

	std::shared_ptr<GpuBuffer> getMatrixBuffer() { return matBuffer; }

	//�R���|�[�l���g���A�^�b�`�������_�Ŏ��s
	void OnAwake() override;

	//�A�^�b�`��̎��̃t���[���J�n���_�Ŏ��s
	void OnStart() override {};

	//�X�V�t�F�[�Y�Ŏ��s
	void OnUpdate() override {};

	//�X�V�t�F�[�Y��Ɏ��s
	void OnLateUpdate() override {};

	//�t���[���I�����Ɏ��s
	void OnFrameEnd() override;
};