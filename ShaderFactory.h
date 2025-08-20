#pragma once

#include<iostream>
#include<string>
#include<fstream>
#include<unordered_map>
#include<array>
#include<list>

#include"VulkanCore.h"

struct Shader;

class ShaderFactory : public std::enable_shared_from_this<ShaderFactory>
{
private:

	VkDevice device;

	//フレームインデックス
	uint32_t frameIndex;

	//作成済みのシェーダモジュールを記録する
	std::unordered_map<std::string, std::weak_ptr<Shader>> shaderStorage;

	//破棄予定のリスト
	std::array<std::list<VkShaderModule>, 2> destructList;

public:

	ShaderFactory(VkDevice& d);

	~ShaderFactory()
	{
		for (int i = 0; i < 2; i++)
		{
			resourceDestruct();
		}

#ifdef _DEBUG
		std::cout << "ShaderFactory :: デストラクタ" << std::endl;
#endif
	}

	//VkShaderModuleを渡す
	std::shared_ptr<Shader> Create(const std::string& shaderPath);

	//テキストファイルを読み込む
	void readFile(std::string shaderPath, std::vector<char>& text);

	//遅延破棄リストにリソースを追加する
	void addDefferedDestruct(VkShaderModule& module);

	//リソースを破棄する
	void resourceDestruct();
};

struct Shader
{
	std::string path;

	VkShaderModule module;

	std::shared_ptr<ShaderFactory> factory;

	Shader(std::shared_ptr<ShaderFactory> f,std::string p)
	{
		module = nullptr;

		factory = f;

		path = p;
	}

	~Shader()
	{
		factory->addDefferedDestruct(module);
	}

	bool operator==(const Shader& other) const
	{
		return path == other.path;
	}
};