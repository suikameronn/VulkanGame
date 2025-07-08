#include"GpuDescriptorSetLayoutFactory.h"

GpuDescriptorSetLayoutFactory::GpuDescriptorSetLayoutFactory(VkDevice& d,std::shared_ptr<GpuDescriptorSetLayoutBuilder> b)
{
	device = d;

	builder = b;

	frameIndex = 1;
}

GpuDescriptorSetLayoutFactory::~GpuDescriptorSetLayoutFactory()
{
	for (auto& itr : layoutStorage)
	{
		if (!itr.second.expired())
		{
			//�{���͂����Ŕj���������̂͂Ȃ��͂�

			itr.second.lock().reset();
		}
	}
}

//���C�A�E�g�̍쐬
std::shared_ptr<DescriptorSetLayout> GpuDescriptorSetLayoutFactory::createLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	//�\���̂̃t�@�N�g���[��ݒ肵�Ă���
	std::shared_ptr<DescriptorSetLayout> layout
		= std::make_shared<DescriptorSetLayout>(shared_from_this());

	//���ۂɃ��C�A�E�g���쐬����
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout->layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	return layout;
}

//����̃��C�A�E�g����r���f�B���O��Ԃ�
void GpuDescriptorSetLayoutFactory::convertBinding(const LayoutPattern& pattern,std::vector<VkDescriptorSetLayoutBinding>& bindings)
{

	if (pattern == LayoutPattern::MVPANIM)
	{
		//���_�V�F�[�_�ƃR���s���[�g�V�F�[�_
		//�s��p�̃��j�t�H�[���o�b�t�@
		//���̃Z�b�g����
		//���ꂼ��mvp�s��ƃA�j���[�V�����s�������

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT);
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT);
	}
	else if (pattern == LayoutPattern::MATERIAL)
	{
		//���ׂẴt���O�����g�V�F�[�_
		//�e�N�X�`���̉e���x�̍\���̂�5���̃e�N�X�`��

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);

		for (int i = 0; i < MAX_TEXTURE_COUNT; i++)
		{
			builder->setProperty(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		}
	}
	else if (pattern == LayoutPattern::LIGHT)
	{
		//�t���O�����g�V�F�[�_
		//���C�g�̐F��ʒu���L�^�������j�t�H�[���o�b�t�@.

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	else if (pattern == LayoutPattern::RAYCAST)
	{
		//���ׂăR���s���[�g�V�F�[�_
		//���_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@���X�g���[�W�o�b�t�@�Ƃ��Č��ѕt����

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT);
		builder->setProperty(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT);
	}
	else if (pattern == LayoutPattern::UI)
	{
		//���_�V�F�[�_�ƃt���O�����g�V�F�[�_
		//UI�̃e�N�X�`���𒣂�t����|���S���̍��W�ϊ��s��Ɖ摜�����ѕt����

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
		builder->setProperty(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	else if (pattern == LayoutPattern::SINGLE_TEX_FLAG)
	{
		//�t���O�����g�V�F�[�_
		//�P��̃e�N�X�`�������ѕt����

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

	}
	else if (pattern == LayoutPattern::SINGLE_UNIFORM_VERT)
	{
		//���_�V�F�[�_
		//�P��̃��j�t�H�[���o�b�t�@�����ѕt����

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
	}
	else if (pattern == LayoutPattern::SINGLE_UNIFORM_FLAG)
	{
		//�t���O�����g�V�F�[�_
		//�P��̃��j�t�H�[���o�b�t�@�����ѕt����

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	//�o�C���f�B���O���쐬����
	builder->Create(bindings);
}

//���C�A�E�g�̍쐬
std::shared_ptr<DescriptorSetLayout> GpuDescriptorSetLayoutFactory::Create(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	if (bindings.size() == 0)
	{
		return std::shared_ptr<DescriptorSetLayout>();
	}

	//����̃��C�A�E�g�����ɍ쐬����Ă��Ȃ����𒲂ׂ�
	std::weak_ptr<DescriptorSetLayout> preCreateLayout = layoutStorage[bindings];
	if (!preCreateLayout.expired())
	{
		return preCreateLayout.lock();
	}

	//���ۂɃ��C�A�E�g���쐬����
	std::shared_ptr<DescriptorSetLayout> layout = createLayout(bindings);

	//�n�b�V���l����������
	layout->hashKey = layoutHash.genHash(bindings);

	//���C�A�E�g��o�^���Ă���
	layoutStorage[bindings] = layout;

	return layout;
}

//����̃��C�A�E�g�̍쐬
std::shared_ptr<DescriptorSetLayout> GpuDescriptorSetLayoutFactory::Create(const LayoutPattern& pattern)
{
	//enum class����o�C���f�B���O�����
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	convertBinding(pattern, bindings);

	//����̃��C�A�E�g�����ɍ쐬����Ă��Ȃ����𒲂ׂ�
	std::weak_ptr<DescriptorSetLayout> preCreateLayout = layoutStorage[bindings];
	if (!preCreateLayout.expired())
	{
		return preCreateLayout.lock();
	}

	//���ۂɃ��C�A�E�g���쐬����
	std::shared_ptr<DescriptorSetLayout> layout = createLayout(bindings);

	//�n�b�V���l����������
	layout->hashKey = layoutHash.genHash(bindings);

	//���C�A�E�g��o�^���Ă���
	layoutStorage[bindings] = layout;

	return layout;
}

//�x���j�����X�g�Ƀ��\�[�X��ǉ�����
void GpuDescriptorSetLayoutFactory::addDefferedDestruct(VkDescriptorSetLayout& layout)
{
	destructList[frameIndex].push_back(layout);
}

//���\�[�X��j������
void GpuDescriptorSetLayoutFactory::resourceDestruct()
{
	//���ۂɃ��\�[�X��j������
	for (auto& layout : destructList[frameIndex])
	{
		vkDestroyDescriptorSetLayout(device, layout, nullptr);
	}

	//�t���[���C���f�b�N�X���X�V����
	frameIndex = (frameIndex == 0) ? 1 : 0;
}