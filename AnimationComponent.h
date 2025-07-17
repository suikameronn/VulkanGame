#pragma once

#include"GltfModelComponent.h"

#include"GpuBufferFactory.h"
#include"DescriptorSetFactory.h"


//�m�[�h���Ƃ̃g�����X�t�H�[�����L�^
struct NodeTransform
{
	std::vector<glm::vec3> translation;
	std::vector<glm::quat> rotation;
	std::vector<glm::vec3> scale;

	std::vector<glm::mat4> matrix;
	std::vector<glm::mat4> nodeTransform;

	void setNodeCount(int nodeCount)
	{
		translation.resize(nodeCount);
		rotation.resize(nodeCount);
		scale.resize(nodeCount);
		matrix.resize(nodeCount);
		nodeTransform.resize(nodeCount);

		std::fill(translation.begin(), translation.end(), glm::vec3(0.0f));
		std::fill(rotation.begin(), rotation.end(), glm::quat());
		std::fill(scale.begin(), scale.end(), glm::vec3(1.0f));
		std::fill(matrix.begin(), matrix.end(), glm::mat4(1.0f));
		std::fill(nodeTransform.begin(), nodeTransform.end(), glm::mat4(1.0f));
	}
};

//�ʏ�̃����_�����O�̃A�j���[�V�����p�̍s��
struct AnimationUBO
{
	alignas(16) glm::mat4 nodeMatrix;
	alignas(16) glm::mat4 matrix;
	alignas(16) std::array<glm::mat4, 128> boneMatrix;
	alignas(16) int boneCount;
};

class AnimationComponent : public IComponent
{
private:

	//�o�b�t�@�̍쐬
	std::shared_ptr<GpuBufferFactory> bufferFactory;
	//�f�B�X�N���v�^�Z�b�g���C�A�E�g�̍쐬
	std::shared_ptr<GpuDescriptorSetLayoutFactory> layoutFactory;
	//�f�B�X�N���v�^�Z�b�g�̍쐬
	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;

	//�v���~�e�B�u���Ƃ�DescriptorSet
	std::vector<std::shared_ptr<DescriptorSet>> descriptorSet;

	//Gltf���f���̃f�[�^
	std::shared_ptr<GltfModel> gltfModel;

	//�m�[�h���Ƃ̃g�����X�t�H�[��
	NodeTransform nodeTransform;

	//�o�ߎ���
	double deltaTime;
	//�A�j���[�V�������Đ����n�߂�����
	double startTime;

	//�A�j���[�V�����̐؂�ւ��t���b�O�A�A�j���[�V�����J�n���Ԃ����Z�b�g
	//����A�j���[�V���������[�v������ہA�A�j���[�V�������I�������Ƃ���true��
	bool animationChange;

	//���ݍĐ����Ă���A�j���[�V�����̖��O
	std::string currentPlayAnimationName;
	//���̍Đ�����\��̃A�j���[�V����
	std::string nextPlayAnimationName;
	//gltf���f���̎��A�j���[�V�����̖��O�̔z��
	std::vector<std::string> animationNames;

	//�A�j���[�V�����p�s��̔z��
	std::vector<std::array<glm::mat4, 128>> jointMatrices;

	//�A�j���[�V�����p�s����L�^���邽�߂̃o�b�t�@
	std::vector<std::shared_ptr<GpuBuffer>> animationBuffer;

	//�A�j���[�V�����p�s������ѕt����f�B�X�N���v�^�Z�b�g
	std::vector<std::shared_ptr<DescriptorSet>> descriptorSet;

	//GPU��̃o�b�t�@���쐬����
	void createBuffer();
	void createBuffer(const GltfNode* node);

	//�f�B�X�N���v�^�Z�b�g�����
	void createDescriptorSet();
	void createDescriptorSet(const GltfNode* node);

public:

	AnimationComponent(std::shared_ptr<GpuBufferFactory> buffer
		, std::shared_ptr<GpuDescriptorSetLayoutFactory> layout
		, std::shared_ptr<DescriptorSetFactory> desc);

	//3D���f����ݒ肷��
	void setGltfModel(std::shared_ptr<GltfModel> model);

	//�A�j���[�V������؂�ւ���
	void switchPlayAnimation(std::string next);

	////����̃X�P���g���̃A�j���[�V�����s����擾
	std::shared_ptr<DescriptorSet> getJointMatrices(int index);

	//�R���|�[�l���g���A�^�b�`�������_�Ŏ��s
	void OnAwake() override {};

	//�A�^�b�`��̎��̃t���[���J�n���_�Ŏ��s
	void OnStart() override;

	//�X�V�t�F�[�Y�Ŏ��s
	void OnUpdate() override {};

	//�X�V�t�F�[�Y��Ɏ��s
	void OnLateUpdate() override {};

	//�t���[���I�����Ɏ��s
	void OnFrameEnd() override;
};