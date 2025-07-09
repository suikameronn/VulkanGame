#include"ShaderFactory.h"

ShaderFactory::ShaderFactory(VkDevice& d)
{
	device = d;

	frameIndex = 1;
}

std::shared_ptr<Shader> ShaderFactory::Create(const std::string& shaderPath)
{
	//すでに作成済みかどうかを調べる
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
		throw std::runtime_error("ShaderFactory :: Create() :: シェーダモジュールの作成に失敗");
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

//遅延破棄リストにリソースを追加する
void ShaderFactory::addDefferedDestruct(VkShaderModule& module)
{
	destructList[frameIndex].push_back(module);
}

//リソースを破棄する
void ShaderFactory::resourceDestruct()
{
	for (auto& module : destructList[frameIndex])
	{
		vkDestroyShaderModule(device, module, nullptr);
	}

	frameIndex = (frameIndex == 0) ? 1 : 0;
}