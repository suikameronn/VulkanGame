#pragma once

#include"GltfModelFactory.h"
#include"Colider.h"

class ColiderFactory
{
private:

	//gltf���f�������o��
	std::shared_ptr<GltfModelFactory> gltfModelFactory;

	std::shared_ptr<GpuBufferFactory> bufferFactory;

	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;

	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;

	//���ɔz��ɑ������\��̃C���f�b�N�X
	size_t lastIndex;

	//�R���C�_�[���L�^���Ă���
	std::vector<std::unique_ptr<Colider>> coliderStorage;

	//�R���C�_�[���j�����ꂽ�ꍇ�́A�󂢂��C���f�b�N�X���W�߂Ă����z��
	std::vector<int> recycleIndex;

public:

	ColiderFactory(std::shared_ptr<GltfModelFactory> gltf, std::shared_ptr<GpuBufferFactory> buffer
		, std::shared_ptr<DescriptorSetLayoutFactory> layout, std::shared_ptr<DescriptorSetFactory> desc);

	size_t Create(const size_t& modelID);

	std::unique_ptr<Colider>& GetColider(const size_t& coliderID);
};