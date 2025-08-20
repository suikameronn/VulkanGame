#include"TextureFactory.h"

TextureFactory::TextureFactory(VkDevice& d, std::shared_ptr<TextureBuilder> b)
{
	builder = b;

	device = d;

	frameIndex = 1;

	copy = std::make_shared<TextureCopy>();

	count = 0;
}

//�v���Z�b�g�̃v���p�e�B���擾
TextureProperty TextureFactory::getPreset(const TexturePattern& pattern)
{
	return convertPattern(pattern);
}

TextureProperty TextureFactory::convertPattern(const uint32_t& width, const uint32_t& height, const TexturePattern& pattern)
{
	if (pattern == TexturePattern::NORMAL)
	{
		//�ʏ��3D���f���ȂǂɎg����e�N�X�`��

		return builder->initProperty()
			->withWidthHeight(width, height) //���ƍ���
			->withFormat(VK_FORMAT_R8G8B8A8_SRGB) //sRGB�t�H�[�}�b�g
			->withNumSamples(VK_SAMPLE_COUNT_1_BIT) //�T���v�����O��
			->withTiling(VK_IMAGE_TILING_OPTIMAL) //�^�C���z�u
			->withUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT) //�g�p���@
			->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) //�f�o�C�X���[�J��������
			->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED) //�������C�A�E�g
			->withFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) //�ŏI���C�A�E�g
			->withViewType(VK_IMAGE_VIEW_TYPE_2D) //�r���[�^�C�v
			->withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT) //�A�X�y�N�g�t���O
			->addView()
			->withLayerCount(1) //���C���[��
			->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR) //�~�b�v�}�b�v���[�h
			->withAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT) //�A�h���X���[�h
			->withMagFilter(VK_FILTER_LINEAR) //�g�厞�̃t�B���^
			->withMinFilter(VK_FILTER_LINEAR) //�k�����̃t�B���^
			->Build();

	}
	else if (pattern == TexturePattern::CUBEMAP)
	{
		//�L���[�u�}�b�v��IBL�p�̃e�N�X�`��

		return builder->initProperty()
			->withWidthHeight(width, height) //���ƍ���
			->withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
			->withNumSamples(VK_SAMPLE_COUNT_1_BIT)
			->withTiling(VK_IMAGE_TILING_OPTIMAL)
			->withUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT
				| VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
			->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			->withFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			->withViewType(VK_IMAGE_VIEW_TYPE_CUBE)
			->withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
			->addView()
			->withLayerCount(6) //�L���[�u�}�b�v�Ȃ̂�6��
			->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
			->withAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
			->withMagFilter(VK_FILTER_LINEAR)
			->withMinFilter(VK_FILTER_LINEAR)
			->Build();
	}
	else if (pattern == TexturePattern::CALC_CUBEMAP)
	{
		//�L���[�u�}�b�v�̌v�Z�p�̃e�N�X�`��
		return builder->initProperty()
			->withWidthHeight(width, height) //���ƍ���
			->withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
			->withNumSamples(VK_SAMPLE_COUNT_1_BIT)
			->withTiling(VK_IMAGE_TILING_OPTIMAL)
			->withUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
			->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			->withFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			->withViewType(VK_IMAGE_VIEW_TYPE_CUBE)
			->withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
			->addView()
			->withLayerCount(1)
			->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
			->withAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) //�v�Z�p�Ȃ̂ŃG�b�W�Ő؂�
			->withMagFilter(VK_FILTER_LINEAR)
			->withMinFilter(VK_FILTER_LINEAR)
			->Build();
	}
}

TextureProperty TextureFactory::convertPattern(const TexturePattern& pattern)
{
	if (pattern == TexturePattern::NORMAL)
	{
		//�ʏ��3D���f���ȂǂɎg����e�N�X�`��

		return builder->initProperty()
			->withFormat(VK_FORMAT_R8G8B8A8_SRGB) //sRGB�t�H�[�}�b�g
			->withNumSamples(VK_SAMPLE_COUNT_1_BIT) //�T���v�����O��
			->withTiling(VK_IMAGE_TILING_OPTIMAL) //�^�C���z�u
			->withUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT) //�g�p���@
			->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) //�f�o�C�X���[�J��������
			->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED) //�������C�A�E�g
			->withFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) //�ŏI���C�A�E�g
			->withViewType(VK_IMAGE_VIEW_TYPE_2D) //�r���[�^�C�v
			->withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT) //�A�X�y�N�g�t���O
			->addView()
			->withLayerCount(1) //���C���[��
			->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR) //�~�b�v�}�b�v���[�h
			->withAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT) //�A�h���X���[�h
			->withMagFilter(VK_FILTER_LINEAR) //�g�厞�̃t�B���^
			->withMinFilter(VK_FILTER_LINEAR) //�k�����̃t�B���^
			->Build();

	}
	else if (pattern == TexturePattern::CUBEMAP)
	{
		//�L���[�u�}�b�v��IBL�p�̃e�N�X�`��

		return builder->initProperty()
			->withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
			->withNumSamples(VK_SAMPLE_COUNT_1_BIT)
			->withTiling(VK_IMAGE_TILING_OPTIMAL)
			->withUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT
				| VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
			->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			->withFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			->withViewType(VK_IMAGE_VIEW_TYPE_CUBE)
			->withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
			->addView()
			->withLayerCount(6) //�L���[�u�}�b�v�Ȃ̂�6��
			->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
			->withAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
			->withMagFilter(VK_FILTER_LINEAR)
			->withMinFilter(VK_FILTER_LINEAR)
			->Build();
	}
	else if (pattern == TexturePattern::CALC_CUBEMAP)
	{
		//�L���[�u�}�b�v�̌v�Z�p�̃e�N�X�`��
		return builder->initProperty()
			->withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
			->withNumSamples(VK_SAMPLE_COUNT_1_BIT)
			->withTiling(VK_IMAGE_TILING_OPTIMAL)
			->withUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
			->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			->withFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			->withViewType(VK_IMAGE_VIEW_TYPE_CUBE)
			->withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
			->addView()
			->withLayerCount(1)
			->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
			->withAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) //�v�Z�p�Ȃ̂ŃG�b�W�Ő؂�
			->withMagFilter(VK_FILTER_LINEAR)
			->withMinFilter(VK_FILTER_LINEAR)
			->Build();
	}
}

TextureProperty TextureFactory::convertPattern(const std::string imageFilePath, const TexturePattern& pattern)
{
	//�摜�T�C�Y���w�肳��Ȃ��ꍇ�́A�O������摜�t�@�C����
	//�ǂݍ��ނ��Ƃ�O��Ƃ���

	if (pattern == TexturePattern::NORMAL)
	{
		//�ʏ��3D���f���ȂǂɎg����e�N�X�`��

		return builder->initProperty()
			->withImageFile(imageFilePath)
			->withFormat(VK_FORMAT_R8G8B8A8_SRGB) //sRGB�t�H�[�}�b�g
			->withNumSamples(VK_SAMPLE_COUNT_1_BIT) //�T���v�����O��
			->withTiling(VK_IMAGE_TILING_OPTIMAL) //�^�C���z�u
			->withUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT) //�g�p���@
			->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) //�f�o�C�X���[�J��������
			->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED) //�������C�A�E�g
			->withFinalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) //�ŏI���C�A�E�g
			->withViewType(VK_IMAGE_VIEW_TYPE_2D) //�r���[�^�C�v
			->withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT) //�A�X�y�N�g�t���O
			->addView()
			->withLayerCount(1) //���C���[��
			->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR) //�~�b�v�}�b�v���[�h
			->withAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT) //�A�h���X���[�h
			->withMagFilter(VK_FILTER_LINEAR) //�g�厞�̃t�B���^
			->withMinFilter(VK_FILTER_LINEAR) //�k�����̃t�B���^
			->Build();

	}
	else if (pattern == TexturePattern::CUBEMAP)
	{
		//�L���[�u�}�b�v��IBL�p�̃e�N�X�`��

		return builder->initProperty()
			->withImageFile(imageFilePath)
			->withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
			->withNumSamples(VK_SAMPLE_COUNT_1_BIT)
			->withTiling(VK_IMAGE_TILING_OPTIMAL)
			->withUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT
				| VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
			->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			->withViewType(VK_IMAGE_VIEW_TYPE_CUBE)
			->withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
			->addView()
			->withLayerCount(6) //�L���[�u�}�b�v�Ȃ̂�6��
			->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
			->withAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT)
			->withMagFilter(VK_FILTER_LINEAR)
			->withMinFilter(VK_FILTER_LINEAR)
			->Build();
	}
	else if (pattern == TexturePattern::CALC_CUBEMAP)
	{
		//�L���[�u�}�b�v�̌v�Z�p�̃e�N�X�`��
		return builder->initProperty()
			->withImageFile(imageFilePath)
			->withFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
			->withNumSamples(VK_SAMPLE_COUNT_1_BIT)
			->withTiling(VK_IMAGE_TILING_OPTIMAL)
			->withUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			->withMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			->withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			->withViewType(VK_IMAGE_VIEW_TYPE_2D)
			->withViewAccess(VK_IMAGE_ASPECT_COLOR_BIT)
			->withTargetLayer(0,1)
			->addView()
			->withLayerCount(1)
			->withMipMapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
			->withAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) //�v�Z�p�Ȃ̂ŃG�b�W�Ő؂�
			->withMagFilter(VK_FILTER_LINEAR)
			->withMinFilter(VK_FILTER_LINEAR)
			->Build();
	}
}

//�O���̉摜�t�@�C������e�N�X�`�������ꍇ�𕪂���
//����̃v���Z�b�g����v���p�e�B��ݒ肷��
std::shared_ptr<Texture> TextureFactory::Create(const std::string& filePath, const TexturePattern& pattern)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(shared_from_this(), count);

	count++;

	TextureProperty property = convertPattern(filePath, pattern);
	texture->property = property;

	texture->viewArray.resize(property.viewArray.size());

	//�r���_�[�ŉ摜�t�@�C������e�N�X�`�����쐬
	builder->Create(property, texture->image
		, texture->memory, texture->viewArray, texture->sampler);

	textureStorage[filePath] = texture;

	return texture;
}

//�摜�f�[�^��z���Ŏ󂯎��
std::shared_ptr<Texture> TextureFactory::Create(const uint32_t& texChannel, const unsigned char* pixels
	, const uint32_t& width, const uint32_t& height, const TexturePattern& pattern)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(shared_from_this(), count);

	count++;

	TextureProperty property = convertPattern(width, height, pattern);
	texture->property = property;

	texture->viewArray.resize(property.viewArray.size());

	//�r���_�[�ŉ摜�t�@�C������e�N�X�`�����쐬
	builder->Create(texChannel, pixels, property
		, texture->image, texture->memory, texture->viewArray, texture->sampler);

	return texture;
}

//�摜�f�[�^�͓��ꂸ�ɁA�e�N�X�`���o�b�t�@�݂̂����
std::shared_ptr<Texture> TextureFactory::Create(const TextureProperty& property)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(shared_from_this(), count);
	texture->property = property;

	count++;

	texture->viewArray.resize(property.viewArray.size());

	//�r���_�[�ŉ摜�t�@�C������e�N�X�`�����쐬
	builder->Create(property, texture->image
		, texture->memory, texture->viewArray, texture->sampler);

	return texture;
}

//�摜�f�[�^�͓��ꂸ�ɁA�e�N�X�`���o�b�t�@�݂̂����
std::shared_ptr<Texture> TextureFactory::Create(const uint32_t& width, const uint32_t& height
	, const TexturePattern& pattern)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(shared_from_this(), count);

	count++;

	TextureProperty property = convertPattern(width, height, pattern);
	texture->property = property;

	texture->viewArray.resize(property.viewArray.size());

	//�r���_�[�ŉ摜�t�@�C������e�N�X�`�����쐬
	builder->Create(property, texture->image
		, texture->memory, texture->viewArray, texture->sampler);

	return texture;
}

//�X���b�v�`�F�[���p�̉摜�ƃr���[���쐬(�T���v���[�͍쐬���Ȃ�)
std::shared_ptr<Texture> TextureFactory::ImageViewCreate(const TextureProperty& property)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(shared_from_this(), count);
	texture->property = property;

	count++;

	texture->viewArray.resize(property.viewArray.size());

	//�r���_�[�ŉ摜�t�@�C������e�N�X�`�����쐬
	builder->Create(property, texture->image, texture->memory, texture->viewArray);

	return texture;
}

//VkImage�݂̂��쐬����AVkImageView�͂̂��ɍ쐬����
std::shared_ptr<Texture> TextureFactory::ImageCreate(const TextureProperty& property)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(shared_from_this(), count);
	texture->property = property;

	count++;

	texture->viewArray.clear();

	//VkImage�݂̂��쐬
	builder->Create(property, texture->image, texture->memory, texture->viewArray);

	return texture;
}

//�摜�f�[�^�͓��ꂸ�ɁA�e�N�X�`���o�b�t�@�݂̂����
std::shared_ptr<Texture> TextureFactory::ViewCreate(const TextureProperty& property, VkImage& image)
{
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(shared_from_this(), count);
	texture->property = property;

	count++;

	texture->viewArray.resize(property.viewArray.size());

	texture->image = image;

	//�r���_�[�ŉ摜����e�N�X�`�����쐬
	builder->Create(property, texture->image, texture->memory, texture->viewArray);

	return texture;
}

//�x���j�����X�g�Ƀ��\�[�X��ǉ�����
void TextureFactory::addDefferedDestruct(VkImage& image, VkDeviceMemory& memory
	, std::vector<VkImageView>& viewArray, VkSampler& sampler)
{
	if (image != VK_NULL_HANDLE)
	{
		destructListImage[frameIndex].push_back(image);
	}

	if (memory != VK_NULL_HANDLE)
	{
		destructListMemory[frameIndex].push_back(memory);
	}

	for (auto& view : viewArray)
	{
		if (view != VK_NULL_HANDLE)
		{
			destructListView[frameIndex].push_back(view);
		}
	}

	if (sampler != VK_NULL_HANDLE)
	{
		destructListSampler[frameIndex].push_back(sampler);
	}
}

//���\�[�X��j������
void TextureFactory::resourceDestruct()
{
	frameIndex = (frameIndex == 0) ? 1 : 0;

	for (VkImage& image : destructListImage[frameIndex])
	{
		vkDestroyImage(device, image, nullptr);
	}

	for (VkDeviceMemory& memory : destructListMemory[frameIndex])
	{
		vkFreeMemory(device, memory, nullptr);
	}

	for (VkImageView& view : destructListView[frameIndex])
	{
		vkDestroyImageView(device, view, nullptr);
	}

	for (VkSampler& sampler : destructListSampler[frameIndex])
	{
		vkDestroySampler(device, sampler, nullptr);
	}

	destructListImage[frameIndex].clear();
	destructListMemory[frameIndex].clear();
	destructListView[frameIndex].clear();
	destructListSampler[frameIndex].clear();
}