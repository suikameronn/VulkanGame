#include"ShaderFactory.h"

ShaderFactory::ShaderFactory(VkDevice& d)
{
	device = d;

	frameIndex = 1;
}

std::shared_ptr<Shader> ShaderFactory::Create(const std::string& shaderPath)
{
	//���łɍ쐬�ς݂��ǂ����𒲂ׂ�
	std::weak_ptr<Shader> preCreate = shaderStorage[shaderPath];
	if (!preCreate.expired())
	{
		return preCreate.lock();
	}

	std::shared_ptr<Shader> shader = std::make_shared<Shader>(shared_from_this(), shaderPath);

	std::vector<char> shaderText;
	readFile(shaderPath, shaderText);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderText.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderText.data());

	if (vkCreateShaderModule(device, &createInfo, nullptr, &shader->module) != VK_SUCCESS) {
		throw std::runtime_error("ShaderFactory :: Create() :: �V�F�[�_���W���[���̍쐬�Ɏ��s");
	}

	shaderStorage[shaderPath] = shader;

	return shader;
}

void ShaderFactory::readFile(std::string shaderPath, std::vector<char>& text)
{
	std::ifstream file(shaderPath, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	text.resize(fileSize);

	file.seekg(0);
	file.read(text.data(), fileSize);

	file.close();
}

//�x���j�����X�g�Ƀ��\�[�X��ǉ�����
void ShaderFactory::addDefferedDestruct(VkShaderModule& module)
{
	destructList[frameIndex].push_back(module);
}

//���\�[�X��j������
void ShaderFactory::resourceDestruct()
{
	frameIndex = (frameIndex == 0) ? 1 : 0;

	for (auto& module : destructList[frameIndex])
	{
		vkDestroyShaderModule(device, module, nullptr);
	}

	destructList[frameIndex].clear();
}