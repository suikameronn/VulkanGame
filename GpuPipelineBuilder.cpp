#include"GpuPipelineBuilder.h"

GpuPipelineBuilder::GpuPipelineBuilder(VkDevice& d, std::shared_ptr<ShaderFactory> sf)
{
	device = d;

	shaderFactory = sf;

	//�v���p�e�B�̏�����
	property.initProperty();
}

void GpuPipelineBuilder::Create(PipelineProperty& p)
{
	if (!property.computeShader)
	{
		property.stages.resize(2);

		property.stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		property.stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		property.stages[0].module = property.vertexShader->module;
		property.stages[0].pName = "main";

		property.stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		property.stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		property.stages[1].module = property.fragmentShader->module;
		property.stages[1].pName = "main";
	}
	else if(property.vertexShader && property.fragmentShader)
	{
		property.stages.resize(1);

		property.stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		property.stages[0].stage = VK_SHADER_STAGE_COMPUTE_BIT;
		property.stages[0].module = property.computeShader->module;
		property.stages[0].pName = "main";
	}
	else
	{
		throw std::runtime_error("GpuPipelineBuilder : Create() : �s���ȃV�F�[�_�p�X�̐ݒ�");
	}

	property.vertexInputState.vertexBindingDescriptionCount = 1;
	property.vertexInputState.pVertexBindingDescriptions = &property.bindingDescription;
	property.vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(property.attributeDescriptions.size());
	property.vertexInputState.pVertexAttributeDescriptions = property.attributeDescriptions.data();

	property.colorBlendState.attachmentCount = static_cast<uint32_t>(property.colorBlendStateArray.size());
	property.colorBlendState.pAttachments = property.colorBlendStateArray.data();

	property.dynamicState.dynamicStateCount = static_cast<uint32_t>(property.dynamicStateArray.size());
	property.dynamicState.pDynamicStates = property.dynamicStateArray.data();

	p = property;

	initProperty();
}

//�p�C�v���C���̐ݒ�̏�����
void GpuPipelineBuilder::initProperty()
{
	property.initProperty();
}

//�p�C�v���C�����C�A�E�g��ݒ肷��
GpuPipelineBuilder GpuPipelineBuilder::withPipelineLayout(const std::shared_ptr<PipelineLayout>& pLayout)
{
	property.pLayout = pLayout;

	return *this;
}

//�����_�[�p�X��ݒ肷��
GpuPipeilneBuilder GpuPipelineBuilder::withRenderPass(const std::shared_ptr<RenderPass>& renderPass)
{
	property.renderPass = renderPass;

	return *this;
}

//���_�V�F�[�_�p�X��ݒ�
GpuPipelineBuilder GpuPipelineBuilder::withVertexShader(const std::string& path)
{
	if (!property.computeShader)
	{
		throw std::runtime_error("GpuPipelineBuilder : ���łɃR���s���[�g�V�F�[�_���o�^����Ă��܂�");
	}

	property.vertexShader = shaderFactory->Create(path);

	return *this;
}
//�t���O�����g�V�F�[�_�p�X��ݒ�
GpuPipelineBuilder GpuPipelineBuilder::withFragmentShader(const std::string& path)
{
	if (!property.computeShader)
	{
		throw std::runtime_error("GpuPipelineBuilder : ���łɃR���s���[�g�V�F�[�_���o�^����Ă��܂�");
	}

	property.fragmentShader = shaderFactory->Create(path);

	return *this;
}
//�R���s���[�g�V�F�[�_
GpuPipelineBuilder GpuPipelineBuilder::withComputeShader(const std::string& path)
{
	if (!property.vertexShader || !property.fragmentShader)
	{
		throw std::runtime_error("GpuPipelineBuilder : ���łɒ��_���t���O�����g�V�F�[�_���o�^����Ă��܂�");
	}

	property.computeShader = shaderFactory->Create(path);

	return *this;
}

//���_�f�[�^�̓ǂݎ�����ݒ�
GpuPipelineBuilder GpuPipelineBuilder::withVertexInput(const uint32_t& binding, const VkVertexInputRate& rate
	, const uint32_t& stride)
{
	property.bindingDescription.binding = binding;
	property.bindingDescription.inputRate = rate;
	property.bindingDescription.stride = stride;

	return *this;
}

//���_�A�g���r���[�g�̒ǉ�
GpuPipelineBuilder GpuPipelineBuilder::addVertexInputAttrib(const uint32_t& location, const uint32_t& binding,
	const VkFormat& format, const uint32_t& offset)
{
	VkVertexInputAttributeDescription attribute{};
	attribute.location = location;
	attribute.binding = binding;
	attribute.format = format;
	attribute.offset = offset;

	property.attributeDescriptions.push_back(attribute);

	return *this;
}

//�v���~�e�B�u�̐ݒ�
GpuPipelineBuilder GpuPipelineBuilder::withTopology(const VkPrimitiveTopology& topology)
{
	property.inputAssemblyState.topology = topology;

	return *this;
}


//�v���~�e�B�u�̕`����@��ݒ�
GpuPipelineBuilder GpuPipelineBuilder::withPolygonMode(const VkPolygonMode& mode)
{
	property.rasterizationState.polygonMode = mode;

	return *this;
}
//���C���̑�����ݒ�
GpuPipelineBuilder GpuPipelineBuilder::withLineWidth(const float& width)
{
	property.rasterizationState.lineWidth = width;

	return *this;
}
//�J�����O���[�h�̐ݒ�
GpuPipelineBuilder GpuPipelineBuilder::withCulModel(const VkCullModeFlags& mode)
{
	property.rasterizationState.cullMode = mode;

	return *this;
}
//�|���S���̕\��������E��肩�����ɐݒ�
GpuPipelineBuilder GpuPipelineBuilder::withFrontFace(const VkFrontFace& face)
{
	property.rasterizationState.frontFace = face;

	return *this;
}
//�f�v�X�o�C�A�X��ݒ肷��
GpuPipelineBuilder GpuPipelineBuilder::enableDepthBias(const bool& isBias)
{
	property.rasterizationState.depthBiasEnable = static_cast<VkBool32>(isBias);

	return *this;
}


//�}���`�T���v�����O�V�F�[�f�B���O��ݒ�
GpuPipelineBuilder GpuPipelineBuilder::enableMultiSampleShading(const bool& mode)
{
	property.multisampleState.sampleShadingEnable = static_cast<VkBool32>(mode);

	return *this;
}
//�Œ�̃T���v�����O�V�F�[�f�B���O�|�C���g�̐���ݒ�
GpuPipelineBuilder GpuPipelineBuilder::withMinSampleShading(const float& min)
{
	property.multisampleState.minSampleShading = min;

	return *this;
}
//�}���`�T���v�����O���s���ۂ̃T���v������ݒ�
GpuPipelineBuilder GpuPipelineBuilder::withRansterizationSamples(const VkSampleCountFlagBits& flag)
{
	property.multisampleState.rasterizationSamples = flag;

	return *this;
}


//�f�v�X�e�X�g��ݒ肷��
GpuPipelineBuilder GpuPipelineBuilder::enableDepthTest(const bool& isEnable)
{
	property.depthStencilState.depthTestEnable = static_cast<VkBool32>(isEnable);

	return *this;
}
//z�o�b�t�@�ւ̏������݂�ݒ肷��
GpuPipelineBuilder GpuPipelineBuilder::enableDepthWrite(const bool& isWrite)
{
	property.depthStencilState.depthWriteEnable = static_cast<VkBool32>(isWrite);

	return *this;
}
//z�l�̔�r�̎d����ݒ肷��
GpuPipelineBuilder GpuPipelineBuilder::withDepthCompare(const VkCompareOp& compare)
{
	property.depthStencilState.depthCompareOp = compare;

	return *this;
}
//z�l�͈̔͂��w�肵�A����ɖ����Ȃ����_�͔j������
GpuPipelineBuilder GpuPipelineBuilder::enableDepthBoundsTest(const float& min, const float& max)
{
	property.depthStencilState.depthBoundsTestEnable = VK_TRUE;
	property.depthStencilState.minDepthBounds = min;
	property.depthStencilState.maxDepthBounds = max;

	return *this;
}
//�X�e���V���e�X�g��ݒ肷��
GpuPipelineBuilder GpuPipelineBuilder::enableStencilTest(const VkStencilOpState& front, const VkStencilOpState& back)
{
	property.depthStencilState.stencilTestEnable = VK_TRUE;
	property.depthStencilState.front = front;
	property.depthStencilState.back = back;

	return *this;
}


//���W�b�N���Z��ݒ�
GpuPipelineBuilder GpuPipelineBuilder::withLogicOp(const VkLogicOp& logic)
{
	property.colorBlendState.logicOpEnable = VK_TRUE;
	property.colorBlendState.logicOp = logic;

	return *this;
}
//�A�^�b�`�����g��ǉ�
GpuPipelineBuilder GpuPipelineBuilder::addColoarAttachment(const VkPipelineColorBlendAttachmentState& attachment)
{
	property.colorBlendStateArray.push_back(attachment);

	return *this;
}
//�u�����h�萔��ݒ�
GpuPipelineBuilder GpuPipelineBuilder::withBlendConstant(const float& r, const float& g, const float& b, const float& a)
{
	property.colorBlendState.blendConstants[0] = r;
	property.colorBlendState.blendConstants[1] = g;
	property.colorBlendState.blendConstants[2] = b;
	property.colorBlendState.blendConstants[3] = a;

	return *this;
}

//���I�ɕύX����X�e�[�g��ςݏグ��
GpuPipelineBuilder GpuPipelineBuilder::addDynamicState(const VkDynamicState& state)
{
	property.dynamicStateArray.push_back(state);

	return *this;
}