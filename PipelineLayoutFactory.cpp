#include"PipelineLayoutFactory.h"

PipelineLayoutFactory::PipelineLayoutFactory(VkDevice& d, std::shared_ptr<PipelineLayoutBuilder> b
	, std::shared_ptr<DescriptorSetLayoutFactory> layoutF)
{
	frameIndex = 1;

	device = d;

	builder = b;

	this->layoutFactory = layoutF;
}

PipelineLayoutFactory::~PipelineLayoutFactory()
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
PipelineLayoutProperty PipelineLayoutFactory::convertLayouts(const PipelineLayoutPattern& pattern)
{
	if (pattern == PipelineLayoutPattern::PBR)
	{
		//PBR�V�F�[�_

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::MODELANIMMAT));//���f���s��
		builder->addLayout(layoutFactory->Create(LayoutPattern::CAMERA));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SHADOWMAP));
		builder->addLayout(layoutFactory->Create(LayoutPattern::MATERIAL));
		builder->addLayout(layoutFactory->Create(LayoutPattern::IBL));

		builder->addPushConstant(sizeof(FragmentParam), VK_SHADER_STAGE_FRAGMENT_BIT);
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

		const std::shared_ptr<DescriptorSetLayout> layout = layoutFactory->Create
		(
			layoutFactory->getBuilder()
			->initProperty()
			.setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT)
			.setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT)
			.setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT)
			.Build()
		);

		builder->addLayout(layout);
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
	}
	else if (pattern == PipelineLayoutPattern::CALC_CUBEMAP)
	{
		//�L���[�u�}�b�v�̌v�Z�p

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
	}
	else if (pattern == PipelineLayoutPattern::CALC_IBL_DIFFUSE)
	{
		//IBL�̃f�B�t���[�Y�̌v�Z�p

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
	}
	else if (pattern == PipelineLayoutPattern::CALC_IBL_SPECULAR)
	{
		//IBL�̃X�y�L�����[�̌v�Z�p

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
		
		builder->addPushConstant(sizeof(SpecularPushConstant), VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	else if (pattern == PipelineLayoutPattern::CALC_IBL_BRDF)
	{
		//IBL��BRDF�v�Z�p

		builder->initProperty();
	}
	else if(pattern == PipelineLayoutPattern::RAYCAST)
	{
		std::shared_ptr<DescriptorSetLayout> raycastLayout = layoutFactory->Create(layoutFactory->getBuilder()
			->initProperty()
			.setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			.setProperty(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			.Build());

		//���C�L���X�g�p
		builder->initProperty();
		builder->addLayout(raycastLayout)
			.addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT))
			.addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT))
			.addLayout(layoutFactory->Create(LayoutPattern::RAYCAST));

		builder->addPushConstant(sizeof(RaycastPushConstant), VK_SHADER_STAGE_COMPUTE_BIT);
	}

	//���C�A�E�g�̍\���̂��擾����
	return builder->Build();
}

//�r���_�[�Ńp�C�v���C�����C�A�E�g���쐬����
std::shared_ptr<PipelineLayout> PipelineLayoutFactory::createLayout(const PipelineLayoutProperty& property)
{
	//�\���̂���VkDescriptorSetLayout�̔z����擾����
	std::vector<VkDescriptorSetLayout> layouts(property.layoutArray.size());
	for (int i = 0; i < layouts.size(); i++)
	{
		layouts[i] = property.layoutArray[i]->layout;
	}

	//�\���̂̃t�@�N�g���[��ݒ肵�Ă���
	std::shared_ptr<PipelineLayout> pLayout
		= std::make_shared<PipelineLayout>(shared_from_this());

	//���ۂɃp�C�v���C�����C�A�E�g���쐬����
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
	pipelineLayoutInfo.pSetLayouts = layouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(property.pushconstantArray.size());
	pipelineLayoutInfo.pPushConstantRanges = property.pushconstantArray.data();

	//�p�C�v���C�����C�A�E�g���쐬����
	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pLayout->pLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	pLayout->layouts = property.layoutArray;
	pLayout->pushconstants = property.pushconstantArray;

	return pLayout;
}

//�p�C�v���C�����C�A�E�g�̍쐬
std::shared_ptr<PipelineLayout> PipelineLayoutFactory::Create(const PipelineLayoutProperty& property)
{
	if (property.layoutArray.size() == 0)
	{
		return std::shared_ptr<PipelineLayout>();
	}

	//���łɍ쐬����Ă��Ȃ������ׂ�
	std::weak_ptr<PipelineLayout> preCreate = pipelineLayoutStorage[property];
	if (!preCreate.expired())
	{
		return preCreate.lock();
	}

	//���ۂɃp�C�v���C�����C�A�E�g���쐬����
	std::shared_ptr<PipelineLayout> layout = createLayout(property);

	//�n�b�V���l����������
	layout->hashKey = pipelineLayoutHash(property);

	//���C�A�E�g��o�^���Ă���
	pipelineLayoutStorage[property] = layout;

	return layout;
}

//�p�C�v���C�����C�A�E�g�̍쐬
std::shared_ptr<PipelineLayout> PipelineLayoutFactory::Create(const PipelineLayoutPattern& pattern)
{
	std::vector<std::shared_ptr<DescriptorSetLayout>> layouts;
	std::vector<VkPushConstantRange> pushConstants;

	const PipelineLayoutProperty& property = convertLayouts(pattern);

	//���łɍ쐬����Ă��Ȃ������ׂ�
	std::weak_ptr<PipelineLayout> preCreate = pipelineLayoutStorage[property];
	if (!preCreate.expired())
	{
		return preCreate.lock();
	}

	//���ۂɃp�C�v���C�����C�A�E�g���쐬����
	std::shared_ptr<PipelineLayout> layout = createLayout(property);

	//�n�b�V���l����������
	layout->hashKey = pipelineLayoutHash(property);

	//���C�A�E�g��o�^���Ă���
	pipelineLayoutStorage[property] = layout;

	return layout;
}

//�x���j�����X�g�Ƀ��\�[�X��ǉ�����
void PipelineLayoutFactory::addDefferedDestruct(VkPipelineLayout& pLayout)
{
	destructList[frameIndex].push_back(pLayout);
}

//���\�[�X��j������
void PipelineLayoutFactory::resourceDestruct()
{
	//�t���[���C���f�b�N�X���X�V����
	frameIndex = (frameIndex == 0) ? 1 : 0;

	//���ۂɃ��\�[�X��j������
	for (auto& playout : destructList[frameIndex])
	{
		vkDestroyPipelineLayout(device, playout, nullptr);
	}

	destructList[frameIndex].clear();
}