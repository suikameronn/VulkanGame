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

	//�t���[���C���f�b�N�X
	uint32_t frameIndex;

	//�쐬�ς݂̃V�F�[�_���W���[�����L�^����
	std::unordered_map<std::string, std::weak_ptr<Shader>> shaderStorage;

	//�j���\��̃��X�g
	std::array<std::list<VkShaderModule>, 2> destructList;

public:

	ShaderFactory(VkDevice& d);

	~ShaderFactory()
	{
		for (int i = 0; i < 2; i++)
		{
			resourceDestruct();
			resourceDestruct();
		}

#ifdef _DEBUG
		std::cout << "ShaderFactory :: �f�X�g���N�^" << std::endl;
#endif
	}

	//VkShaderModule��n��
	std::shared_ptr<Shader> Create(const std::string& shaderPath);

	//�e�L�X�g�t�@�C����ǂݍ���
	void readFile(std::string shaderPath, std::vector<char>& text);

	//�x���j�����X�g�Ƀ��\�[�X��ǉ�����
	void addDefferedDestruct(VkShaderModule& module);

	//���\�[�X��j������
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