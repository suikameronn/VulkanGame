#include"GpuPipelineLayoutFactory.h"

GpuPipelineLayoutFactory::GpuPipelineLayoutFactory(VkDevice& d, std::shared_ptr<GpuPipelineLayoutBuilder> b
	, std::shared_ptr<GpuDescriptorSetLayoutFactory> layoutF)
{
	device = d;

	builder = b;

	this->layoutFactory = layoutF;
}

GpuPipelineLayoutFactory::~GpuPipelineLayoutFactory()
{
	for (auto& itr : pipelineLayoutStorage)
	{
		if (!itr.second.expired())
		{
			//�{���͂����Ŕj���������̂͂Ȃ��͂�

			itr.second.lock().reset();
		}
	}
}

//����̃��C�A�E�g����\���̂�ݒ肷��
void GpuPipelineLayoutFactory::convertLayouts(PipelineLayoutPattern pattern
	, std::vector<std::shared_ptr<DescriptorSetLayout>>& layouts, std::vector<VkPushConstantRange>& pushConstant)
{
	if (pattern == PipelineLayoutPattern::PBR)
	{
		//PBR�V�F�[�_

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::MVPANIM));
		builder->addLayout(layoutFactory->Create(LayoutPattern::MATERIAL));
		builder->addLayout(layoutFactory->Create(LayoutPattern::LIGHT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::LIGHT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));

		VkPushConstantRange p1{};
		p1.offset = 0;
		p1.size = sizeof(FragmentParam);
		p1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		pushConstant.push_back(p1);
	}
	else if (pattern == PipelineLayoutPattern::UI)
	{
		//UI�����_�����O�p

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
	}
	else if (pattern == PipelineLayoutPattern::CUBEMAP)
	{
		//�L���[�u�}�b�v�����_�����O

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
	}
	else if (pattern == PipelineLayoutPattern::CALC_SHADOWMAP)
	{
		//�V���h�E�}�b�v�̌v�Z�p

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::MVPANIM));
	}
	else if (pattern == PipelineLayoutPattern::CALC_CUBEMAP)
	{
		//�L���[�u�}�b�v�̌v�Z�p

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
	}
	else if (pattern == PipelineLayoutPattern::CALCIBL_DIFFUSE_SPECULAR)
	{
		//IBL�̃f�B�t���[�Y�ƃX�y�L�����[�̌v�Z�p

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
	}
	else if (pattern == PipelineLayoutPattern::CALCIBL_BRDF)
	{
		//IBL��BRDF�v�Z�p

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
	}

	//���C�A�E�g�̍\���̂��擾����
	builder->Create(layouts);
}

//�r���_�[�Ńp�C�v���C�����C�A�E�g���쐬����
std::shared_ptr<PipelineLayout> GpuPipelineLayoutFactory::createLayout(std::vector<std::shared_ptr<DescriptorSetLayout>>& layoutStruct
	, std::vector<VkPushConstantRange>& pushConstants)
{
	//�\���̂���VkDescriptorSetLayout�̔z����擾����
	std::vector<VkDescriptorSetLayout> layouts(layoutStruct.size());
	for (int i = 0; i < layouts.size(); i++)
	{
		layouts[i] = layoutStruct[i]->layout;
	}

	//�\���̂̃t�@�N�g���[��ݒ肵�Ă���
	std::shared_ptr<PipelineLayout> pLayout
		= std::make_shared<PipelineLayout>(shared_from_this());

	//���ۂɃp�C�v���C�����C�A�E�g���쐬����
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
	pipelineLayoutInfo.pSetLayouts = layouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
	pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();

	//�p�C�v���C�����C�A�E�g���쐬����
	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pLayout->pLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	pLayout->layouts = layoutStruct;

	return pLayout;
}

//�p�C�v���C�����C�A�E�g�̍쐬
std::shared_ptr<PipelineLayout> GpuPipelineLayoutFactory::Create(std::vector<std::shared_ptr<DescriptorSetLayout>>& layouts
	, std::vector<VkPushConstantRange>& pushConstants)
{
	if (layouts.size() == 0)
	{
		return std::shared_ptr<PipelineLayout>();
	}

	//���łɍ쐬����Ă��Ȃ������ׂ�
	std::weak_ptr<PipelineLayout> preCreate = pipelineLayoutStorage[{layouts, pushConstants}];
	if (!preCreate.expired())
	{
		return preCreate.lock();
	}

	//���ۂɃp�C�v���C�����C�A�E�g���쐬����
	std::shared_ptr<PipelineLayout> layout = createLayout(layouts, pushConstants);

	//�n�b�V���l����������
	layout->hashKey = pipelineLayoutHash({ layouts, pushConstants });

	//���C�A�E�g��o�^���Ă���
	pipelineLayoutStorage[{layouts, pushConstants}] = layout;

	return layout;
}

//�p�C�v���C�����C�A�E�g�̍쐬
std::shared_ptr<PipelineLayout> GpuPipelineLayoutFactory::Create(PipelineLayoutPattern pattern)
{
	std::vector<std::shared_ptr<DescriptorSetLayout>> layouts;
	std::vector<VkPushConstantRange> pushConstants;

	convertLayouts(pattern, layouts, pushConstants);

	//���łɍ쐬����Ă��Ȃ������ׂ�
	std::weak_ptr<PipelineLayout> preCreate = pipelineLayoutStorage[{layouts, pushConstants}];
	if (!preCreate.expired())
	{
		return preCreate.lock();
	}

	//���ۂɃp�C�v���C�����C�A�E�g���쐬����
	std::shared_ptr<PipelineLayout> layout = createLayout(layouts, pushConstants);

	//�n�b�V���l����������
	layout->hashKey = pipelineLayoutHash({ layouts,pushConstants });

	//���C�A�E�g��o�^���Ă���
	pipelineLayoutStorage[{layouts, pushConstants}] = layout;

	return layout;
}

//�x���j�����X�g�Ƀ��\�[�X��ǉ�����
void GpuPipelineLayoutFactory::addDefferedDestruct(VkPipelineLayout& pLayout)
{
	destructList[frameIndex].push_back(pLayout);
}

//���\�[�X��j������
void GpuPipelineLayoutFactory::resourceDestruct()
{
	//���ۂɃ��\�[�X��j������
	for (auto& playout : destructList[frameIndex])
	{
		vkDestroyPipelineLayout(device, playout, nullptr);
	}

	//�t���[���C���f�b�N�X���X�V����
	frameIndex = (frameIndex == 0) ? 1 : 0;
}