#pragma once

#include"GltfModel.h"

#include"IComponent.h"

#include"GpuBufferFactory.h"
#include"DescriptorSetFactory.h"

class GltfModelComponent : public IComponent
{
private:

	//�o�b�t�@�̍쐬
	std::shared_ptr<GpuBufferFactory> bufferFactory;
	//�f�B�X�N���v�^�Z�b�g���C�A�E�g�̍쐬
	std::shared_ptr<GpuDescriptorSetLayoutFactory> layoutFactory;
	//�f�B�X�N���v�^�Z�b�g�̍쐬
	std::shared_ptr<DescriptorSet> descriptorSetFactory;

	//Gltf���f���̃f�[�^
	std::shared_ptr<GltfModel> gltfModel;

public:

	GltfModelComponent(std::shared_ptr<GpuBufferFactory> buffer
		, std::shared_ptr<GpuDescriptorSetLayoutFactory> layout
		, std::shared_ptr<DescriptorSet> desc
		, std::shared_ptr<GltfModel> model);

	//Gltf���f����Ԃ�
	std::shared_ptr<GltfModel> getGltfModel() { return gltfModel; }

	//3D���f���̒��S�����߂�
	glm::vec3 getCenter();

	//�R���|�[�l���g���A�^�b�`�������_�Ŏ��s
	void OnAwake() override;

	//�A�^�b�`��̎��̃t���[���J�n���_�Ŏ��s
	void OnStart() override {};

	//�X�V�t�F�[�Y�Ŏ��s
	void OnUpdate() override {};

	//�X�V�t�F�[�Y��Ɏ��s
	void OnLateUpdate() override {};

	//�t���[���I�����Ɏ��s
	void OnFrameEnd() override {};
};