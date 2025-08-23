#include"TextureBuilder.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "tiny_gltf.h"

TextureBuilder::TextureBuilder(std::shared_ptr<VulkanCore> core,std::shared_ptr<GpuBufferFactory> buffer
	, std::shared_ptr<CommandBufferFactory> command)
{
	vulkanCore = core;
	bufferFactory = buffer;
	commandBufferFactory = command;

	device = vulkanCore->getLogicDevice();
}

//�v���p�e�B�̏�����
std::shared_ptr<TextureBuilder> TextureBuilder::initProperty()
{
	property.initProperty();

	image.initProperty();
	view.initProperty();
	sampler.initProperty();

	return shared_from_this();
}

//�e�N�X�`���T�C�Y�̐ݒ�
std::shared_ptr<TextureBuilder> TextureBuilder::withWidthHeight(const uint32_t& width, const uint32_t& height)
{
	property.image.info.extent.width = width;
	property.image.info.extent.height = height;

	property.image.info.mipLevels = std::floor(std::log2(std::max(width, height))) + 1;

	return shared_from_this();
}

//�e�N�X�`���T�C�Y�̐ݒ�
std::shared_ptr<TextureBuilder> TextureBuilder::withWidthHeight(const uint32_t& width, const uint32_t& height, const uint32_t& mipmapLevel)
{
	property.image.info.extent.width = width;
	property.image.info.extent.height = height;

	property.image.info.mipLevels = mipmapLevel;

	return shared_from_this();
}

//�摜�t�@�C������s�N�Z����ǂݎ��
std::shared_ptr<TextureBuilder> TextureBuilder::withImageFile(const std::string filePath)
{
	//�O���̉摜�t�@�C������e�N�X�`�������

	if (!stbi_is_hdr(filePath.c_str()))
	{
		unsigned char* pixels;

		loadImageFile(filePath, property.image.info.extent.width, property.image.info.extent.height,
			pixels);

		property.image.pixelSize = sizeof(unsigned char);
	}
	else
	{
		float* pixels;

		loadImageFile(filePath, property.image.info.extent.width, property.image.info.extent.height,
			pixels);

		property.image.pixels = pixels;

		property.image.pixelSize = sizeof(float);
	}

	const VkExtent3D extent = property.image.info.extent;

	property.image.info.mipLevels = std::floor(std::log2(std::max(extent.width, extent.height))) + 1;

	return shared_from_this();
}

//�摜�t�@�C������s�N�Z����ǂݎ��
std::shared_ptr<TextureBuilder> TextureBuilder::withImageFile(const std::string filePath, const uint32_t miplevels)
{
	//�O���̉摜�t�@�C������e�N�X�`�������

	if (!stbi_is_hdr(filePath.c_str()))
	{
		loadImageFile(filePath, property.image.info.extent.width, property.image.info.extent.height,
			*static_cast<unsigned char**>(property.image.pixels));

		property.image.pixelSize = sizeof(unsigned char);
	}
	else
	{
		loadImageFile(filePath, property.image.info.extent.width, property.image.info.extent.height,
			*static_cast<float**>(property.image.pixels));

		property.image.pixelSize = sizeof(float);
	}

	property.image.info.mipLevels = miplevels;

	return shared_from_this();
}

//�t�H�[�}�b�g��ݒ�
std::shared_ptr<TextureBuilder> TextureBuilder::withFormat(const VkFormat& format)
{
	property.image.info.format = format;

	return shared_from_this();
}

//VkImageType��ݒ�
std::shared_ptr<TextureBuilder> TextureBuilder::withImageType(const VkImageType& type)
{
	property.image.info.imageType = type;

	return shared_from_this();
}

//VkImageFlag��ݒ�(SamplerCube�̍쐬���ȂǂɎg�p)
std::shared_ptr<TextureBuilder> TextureBuilder::withImageFlag(const VkImageCreateFlagBits& flag)
{
	property.image.info.flags = flag;

	return shared_from_this();
}

//�}���`�T���v�����O���̐ݒ�
std::shared_ptr<TextureBuilder> TextureBuilder::withNumSamples(const VkSampleCountFlagBits& numSamples)
{
	property.image.info.samples = numSamples;

	return shared_from_this();
}

//�s�N�Z���̔z�u��ݒ�
std::shared_ptr<TextureBuilder> TextureBuilder::withTiling(const VkImageTiling& tiling)
{
	property.image.info.tiling = tiling;

	return shared_from_this();
}

//�e�N�X�`���̃o�b�t�@�̎g������ݒ�
std::shared_ptr<TextureBuilder> TextureBuilder::withUsage(const VkImageUsageFlags& usage)
{
	property.image.info.usage = usage;

	return shared_from_this();
}

//�������z�u��ݒ�
std::shared_ptr<TextureBuilder> TextureBuilder::withMemoryProperty(const VkMemoryPropertyFlags& prop)
{
	property.image.memProperty = prop;

	return shared_from_this();
}

//�����̃e�N�X�`���̃��C�A�E�g
std::shared_ptr<TextureBuilder> TextureBuilder::withInitialLayout(const VkImageLayout& layout)
{
	property.image.info.initialLayout = layout;

	return shared_from_this();
}

//�ŏI�I�ȃe�N�X�`���̃��C�A�E�g
std::shared_ptr<TextureBuilder> TextureBuilder::withFinalLayout(const VkImageLayout& layout)
{
	property.image.finalLayout = layout;

	return shared_from_this();
}


//�r���[�̃^�C�v��ݒ肷��
std::shared_ptr<TextureBuilder> TextureBuilder::withViewType(const VkImageViewType& type)
{
	view.info.viewType = type;

	return shared_from_this();
}

//�r���[���A�N�Z�X�ł���f�[�^��ݒ�
std::shared_ptr<TextureBuilder> TextureBuilder::withViewAccess(const VkImageAspectFlags& flag)
{
	view.info.subresourceRange.aspectMask = flag;

	return shared_from_this();
}

//�r���[���A�N�Z�X���郌�C���[�͈̔͂��w�肷��
std::shared_ptr<TextureBuilder> TextureBuilder::withTargetLayer(const uint32_t baseLayer, const uint32_t tagetLayerCount)
{
	view.info.subresourceRange.baseArrayLayer = baseLayer;
	view.info.subresourceRange.layerCount = tagetLayerCount;

	return shared_from_this();
}

//�r���[���A�N�Z�X����~�b�v�}�b�v���x�����w�肷��
std::shared_ptr<TextureBuilder> TextureBuilder::withTargetMipmapLevel(const uint32_t baseMipmapLevel, const uint32_t levelCount)
{
	view.info.subresourceRange.baseMipLevel = baseMipmapLevel;
	view.info.subresourceRange.levelCount = levelCount;

	return shared_from_this();
}

//�r���[��ςݏグ��
std::shared_ptr<TextureBuilder> TextureBuilder::addView()
{
	property.viewArray.push_back(view);

	view.initProperty();

	return shared_from_this();
}

//�e�N�X�`���̃��C���[����ݒ肷��
std::shared_ptr<TextureBuilder> TextureBuilder::withLayerCount(const uint32_t& layerCount)
{
	property.image.info.arrayLayers = layerCount;

	return shared_from_this();
}


//�T���v���[�̃~�b�v�}�b�v���x���Ԃ̕�ԕ��@��ݒ肷��
std::shared_ptr<TextureBuilder> TextureBuilder::withMipMapMode(const VkSamplerMipmapMode& mode)
{
	property.sampler.info.mipmapMode = mode;

	return shared_from_this();
}

//�e�N�X�`���̋��E�����̏�����ݒ�
std::shared_ptr<TextureBuilder> TextureBuilder::withAddressMode(const VkSamplerAddressMode& mode)
{
	property.sampler.info.addressModeU = mode;
	property.sampler.info.addressModeV = mode;
	property.sampler.info.addressModeW = mode;

	return shared_from_this();
}

//�e�N�X�`���̊g�厞�̕�ԕ��@��ݒ肷��
std::shared_ptr<TextureBuilder> TextureBuilder::withMagFilter(const VkFilter& filter)
{
	property.sampler.info.magFilter = filter;

	return shared_from_this();
}

//�e�N�X�`���̏k�����̕�ԕ��@��ݒ肷��
std::shared_ptr<TextureBuilder> TextureBuilder::withMinFilter(const VkFilter& filter)
{
	property.sampler.info.minFilter = filter;

	return shared_from_this();
}

TextureProperty TextureBuilder::Build()
{
	for (auto& view : property.viewArray)
	{
		view.info.format = property.image.info.format;

		if (view.info.subresourceRange.levelCount == 0)
		{
			view.info.subresourceRange.levelCount = property.image.info.mipLevels;
		}

		if (view.info.subresourceRange.layerCount == 0)
		{
			view.info.subresourceRange.layerCount = property.image.info.arrayLayers;
		}
	}

	property.sampler.info.maxLod = static_cast<float>(property.image.info.mipLevels) - 1.0f;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(vulkanCore->getPhysicalDevice(), &properties);
	property.sampler.info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	if (property.image.info.mipLevels == 1)
	{
		property.sampler.info.minLod = 0.0f;
		property.sampler.info.maxLod = 0.0f;
		property.sampler.info.mipLodBias = 0.0f;
	}

	return property;
}

//�O���̉摜��ǂݍ���
void TextureBuilder::loadImageFile(const std::string& filePath, uint32_t& width, uint32_t& height
	,unsigned char*& pixels)
{
	//�ʏ�̃e�N�X�`���̉摜��ǂݍ���
	int texWidth, texHeight, texChannels;
	unsigned char* srcPixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, 0);
	if (!pixels)
	{
		throw std::runtime_error("�摜�̓ǂݍ��݂̎��s: " + filePath);
	}
	width = static_cast<uint32_t>(texWidth);
	height = static_cast<uint32_t>(texHeight);

	if (texChannels == 4)//�摜�ɓ����x�̃`�����l��������ꍇ�́A���̂܂�
	{
		pixels = new unsigned char[width * height * texChannels];
		memcpy(pixels, srcPixels, (width * height * texChannels) - 1);
	}
	else if (texChannels == 3)//�摜�ɓ����x�̃`�����l���������ꍇ�́A���ׂẴs�N�Z���̓����x��255�Ƃ���
		//�����Ƀ`�����l����4�� gpu�ł̃e�N�X�`���쐬�̓s����
	{
		pixels = new unsigned char[width * height * 4];
		memset(pixels, 255, width * height * 4);

		for (uint32_t i = 0; i < width * height; i++)
		{
			pixels[i * 4] = srcPixels[i * 3];
			pixels[i * 4 + 1] = srcPixels[i * 3 + 1];
			pixels[i * 4 + 2] = srcPixels[i * 3 + 2];
		}
	}
	else if (texChannels == 1)
	{
		pixels = new unsigned char[width * height * 4];
		memset(pixels, 255, width * height * 4);

		for (uint32_t i = 0; i < width * height; i++)
		{
			pixels[i * 4] = srcPixels[i];
		}
	}

	stbi_image_free(srcPixels);
}

void TextureBuilder::loadImageFile(const std::string& filePath, uint32_t& width, uint32_t& height
	, float*& pixels)
{
	//HDR�̃e�N�X�`���̉摜��ǂݍ���
	int texWidth, texHeight, texChannels;
	float* srcPixels = stbi_loadf(filePath.c_str(), &texWidth, &texHeight, &texChannels, 0);

	if (!srcPixels)
	{
		throw std::runtime_error("�摜�̓ǂݍ��݂̎��s: " + filePath);
	}
	width = static_cast<uint32_t>(texWidth);
	height = static_cast<uint32_t>(texHeight);

	if (texChannels == 4)//�摜�ɓ����x�̃`�����l��������ꍇ�́A���̂܂�
	{
		pixels = new float[width * height * texChannels];
		memcpy(pixels, srcPixels, (width * height * texChannels) - 1);
	}
	else if (texChannels == 3)//�摜�ɓ����x�̃`�����l���������ꍇ�́A���ׂẴs�N�Z���̓����x��255�Ƃ���
		//�����Ƀ`�����l����4�� gpu�ł̃e�N�X�`���쐬�̓s����
	{
		pixels = new float[width * height * 4];
		memset(pixels, 255.0f, width * height * 4);

		for (uint32_t i = 0; i < width * height; i++)
		{
			pixels[i * 4] = srcPixels[i * 3];
			pixels[i * 4 + 1] = srcPixels[i * 3 + 1];
			pixels[i * 4 + 2] = srcPixels[i * 3 + 2];
		}
	}
	else if (texChannels == 1)
	{
		pixels = new float[width * height * 4];
		memset(pixels, 255.0f, width * height * 4);

		for (uint32_t i = 0; i < width * height; i++)
		{
			pixels[i * 4] = srcPixels[i];
		}
	}

	stbi_image_free(srcPixels);
}

//vkCreateImage���Ăяo��
void TextureBuilder::createVkImageAndMemory(const TextureImageProperty& property, VkImage& image, VkDeviceMemory& memory)
{
	if (vkCreateImage(device, &property.info, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("TextureBuilder : createVkImageAndMemory() : �e�N�X�`���̍쐬�Ɏ��s");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = vulkanCore->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) 
	{
		throw std::runtime_error("TextureBuilder : createVkImageAndMemory() : �e�N�X�`���̃o�b�t�@�̊m�ۂɎ��s");
	}

	vkBindImageMemory(device, image, memory, 0);
}

//�摜�̃��C�A�E�g��ύX����
void TextureBuilder::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout
	, uint32_t mipLevels, uint32_t layerCount)
{
	std::shared_ptr<CommandBuffer> commandBuffer = std::make_shared<CommandBuffer>(vulkanCore->getLogicDevice(), commandBufferFactory)
		->setCommandBufffer(commandBufferFactory->createCommandBuffer(1))
		->setFence(commandBufferFactory->createFence());

	for (uint32_t i = 0; i < mipLevels; i++)
	{
		for (uint32_t j = 0; j < layerCount; j++)
		{
			commandBuffer->recordBegin();

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange.baseMipLevel = i;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = j;
			barrier.subresourceRange.layerCount = 1;

			if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

				if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT)
				{
					barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				}
			}
			else {
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			}

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			//���C�A�E�g���o�͐�ɕύX����
			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			//���C�A�E�g�������_�����O�̏o�͐�ɂ���
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			//���C�A�E�g���V�F�[�_����ǂݎ�邽�߂̂��̂ɂ���
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			//�o�͂��ꂽ�e�N�X�`�����V�F�[�_��ŗ��p�ł���悤�ɂ���
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			//�f�v�X�C���[�W�p�ɂ���
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			//�J���[�A�^�b�`�����g�p�̃��C�A�E�g�𑗐M�p�̃��C�A�E�g�ɕύX����
			else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			//�J���[�A�^�b�`�����g�p�̃��C�A�E�g���V�F�[�_�Ŏg�����߂̃��C�A�E�g�ɂ���
			else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			//�J���[�A�^�b�`�����g�p�̃��C�A�E�g��\���p�̃��C�A�E�g�ɂ���
			else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
			{
				barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				barrier.dstAccessMask = 0;

				sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

				//�p�C�v���C���̏����̂��ׂĂ��I�������Ƃ�
				destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			}
			//�\���p�̃��C�A�E�g���J���[�A�^�b�`�����g�p�̃��C�A�E�g�ɂ���
			else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			{
				barrier.srcAccessMask = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else {
				throw std::invalid_argument("unsupported layout transition!");
			}

			vkCmdPipelineBarrier(
				commandBuffer->getCommand(),
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			commandBuffer->recordEnd();
			commandBuffer->Submit(vulkanCore->getGraphicsQueue());
			commandBuffer->waitFence();
		}
	}

}

void TextureBuilder::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout
	, uint32_t mipLevels, uint32_t layerCount, std::shared_ptr<CommandBuffer> commandBuffer)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layerCount;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT)
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	//���C�A�E�g���o�͐�ɕύX����
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	//���C�A�E�g�������_�����O�̏o�͐�ɂ���
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	//���C�A�E�g���V�F�[�_����ǂݎ�邽�߂̂��̂ɂ���
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	//�o�͂��ꂽ�e�N�X�`�����V�F�[�_��ŗ��p�ł���悤�ɂ���
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	//�f�v�X�C���[�W�p�ɂ���
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	//�J���[�A�^�b�`�����g�p�̃��C�A�E�g�𑗐M�p�̃��C�A�E�g�ɕύX����
	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	//�J���[�A�^�b�`�����g�p�̃��C�A�E�g���V�F�[�_�Ŏg�����߂̃��C�A�E�g�ɂ���
	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	//�J���[�A�^�b�`�����g�p�̃��C�A�E�g��\���p�̃��C�A�E�g�ɂ���
	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = 0;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		//�p�C�v���C���̏����̂��ׂĂ��I�������Ƃ�
		destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	}
	//�\���p�̃��C�A�E�g���J���[�A�^�b�`�����g�p�̃��C�A�E�g�ɂ���
	else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer->getCommand(),
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
}

//�o�b�t�@����VkImage�ɉ摜�f�[�^���R�s�[
void TextureBuilder::copyBufferToImage(const std::shared_ptr<GpuBuffer> staging, VkImage& image
	, const TextureImageProperty& imageProperty)
{
	VkCommandBuffer commandBuffer = vulkanCore->beginSingleTimeCommandBuffer();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = imageProperty.info.arrayLayers;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		imageProperty.info.extent.width,
		imageProperty.info.extent.height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, staging->buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	vulkanCore->endSingleTimeCommandBuffer(commandBuffer);
}


//�~�b�v�}�b�v���x�����쐬
void TextureBuilder::generateMipmaps(VkImage image, const TextureImageProperty& imageProperty)
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(vulkanCore->getPhysicalDevice(), imageProperty.info.format, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) 
	{
		throw std::runtime_error("TextureBuilder : generateMipmaps() : �t�H�[�}�b�g�ɓK�����Ă��Ȃ�");
	}

	VkCommandBuffer commandBuffer = vulkanCore->beginSingleTimeCommandBuffer();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = imageProperty.info.arrayLayers;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = imageProperty.info.extent.width;
	int32_t mipHeight = imageProperty.info.extent.height;

	for (uint32_t i = 1; i < imageProperty.info.mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr
			, 0, nullptr,
			1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0,0,0 };
		blit.srcOffsets[1] = { mipWidth,mipHeight,1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = imageProperty.info.arrayLayers;
		blit.dstOffsets[0] = { 0,0,0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1,1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = imageProperty.info.arrayLayers;

		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = imageProperty.info.mipLevels;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	vulkanCore->endSingleTimeCommandBuffer(commandBuffer);
}

//gpu��ɉ摜�f�[�^��W�J
void TextureBuilder::createImage(const size_t& pixelSize, const void* pixels
	,const TextureImageProperty& imageProperty, VkImage& image, VkDeviceMemory& memory)
{
	VkDeviceSize bufferSize = imageProperty.info.extent.width * imageProperty.info.extent.height * pixelSize * RGBA;

	//�X�e�[�W���O�o�b�t�@��p��
	std::shared_ptr<GpuBuffer> staging = bufferFactory->Create(bufferSize,pixels
		, VK_BUFFER_USAGE_TRANSFER_SRC_BIT
		, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	//�R�s�[���VkImage���쐬
	createVkImageAndMemory(imageProperty, image, memory);

	//�R�s�[���VkImage�̃��C�A�E�g��ύX
	transitionImageLayout(image, imageProperty.info.format, imageProperty.info.initialLayout,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageProperty.info.mipLevels, imageProperty.info.arrayLayers);

	//�X�e�[�W���O�o�b�t�@����VkImage�ɉ摜�f�[�^���R�s�[
	copyBufferToImage(staging, image, imageProperty);

	//�~�b�v�}�b�v���x�����쐬
	generateMipmaps(image, imageProperty);
}

//gpu��ɉ摜�f�[�^��W�J
void TextureBuilder::createImage(const TextureImageProperty& imageProperty, VkImage& image, VkDeviceMemory& memory)
{
	VkDeviceSize bufferSize = imageProperty.info.extent.width * imageProperty.info.extent.height * sizeof(unsigned char) * RGBA;

	//�R�s�[���VkImage���쐬
	createVkImageAndMemory(imageProperty, image, memory);

	if (imageProperty.finalLayout != VK_IMAGE_LAYOUT_UNDEFINED)
	{
		//�R�s�[���VkImage�̃��C�A�E�g��ύX
		transitionImageLayout(image, imageProperty.info.format, imageProperty.info.initialLayout,
			imageProperty.finalLayout, imageProperty.info.mipLevels, imageProperty.info.arrayLayers);
	}
}

//���̃e�N�X�`���̃r���[���쐬
void TextureBuilder::createImageView(const TextureViewProperty& viewProperty, VkImage& image,VkImageView& view)
{
	VkImageViewCreateInfo info = viewProperty.info;

	info.image = image;

	if (vkCreateImageView(device, &info, nullptr, &view) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}
}

//�T���v���[�̍쐬
void TextureBuilder::createSampler(const TextureSamplerProperty& samplerProperty, VkSampler& sampler)
{
	if (vkCreateSampler(device, &samplerProperty.info, nullptr, &sampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create image sampler");
	}
}

void TextureBuilder::Create(const TextureProperty& property
	, VkImage& image,VkDeviceMemory& memory, std::vector<VkImageView>& viewArray,VkSampler& sampler)
{
	if (property.image.pixels)
	{
		//�摜�t�@�C����ǂݍ���ł���ꍇ

		//gpu��ɉ摜�f�[�^��W�J
		createImage(property.image.pixelSize, property.image.pixels, property.image, image, memory);

		for (int i = 0; i < property.viewArray.size(); i++)
		{
			//���̃e�N�X�`���̃r���[���쐬
			createImageView(property.viewArray[i], image, viewArray[i]);
		}

		//�T���v���[�̍쐬
		createSampler(property.sampler, sampler);

		if (property.image.finalLayout != VK_IMAGE_LAYOUT_UNDEFINED)
		{
			//�摜�̃��C�A�E�g���w��̂��̂ɕύX
			transitionImageLayout(image, property.image.info.format, property.image.finalLayout,
				property.image.finalLayout, property.image.info.mipLevels, property.image.info.arrayLayers);
		}

		//�摜�t�@�C���̃s�N�Z���f�[�^���J��
		delete[] property.image.pixels;
	}
	else
	{
		//gpu��ɉ摜�f�[�^��W�J
		createImage(property.image, image, memory);

		for (int i = 0; i < property.viewArray.size(); i++)
		{
			//���̃e�N�X�`���̃r���[���쐬
			createImageView(property.viewArray[i], image, viewArray[i]);
		}

		//�T���v���[�̍쐬
		createSampler(property.sampler, sampler);
	}

}

void TextureBuilder::Create(const uint32_t& texChannel, const unsigned char* pixels, const TextureProperty& property
	, VkImage& image, VkDeviceMemory& memory, std::vector<VkImageView>& viewArray, VkSampler& sampler)
{
	//�s�N�Z���̔z�񂩂炩��e�N�X�`�������
	size_t pixelSize = sizeof(unsigned char);

	//gpu��ɉ摜�f�[�^��W�J
	createImage(pixelSize, pixels, property.image, image, memory);

	for (int i = 0; i < property.viewArray.size(); i++)
	{
		//���̃e�N�X�`���̃r���[���쐬
		createImageView(property.viewArray[i], image, viewArray[i]);
	}

	//�T���v���[�̍쐬
	createSampler(property.sampler, sampler);
}

//�X���b�v�`�F�[���p
void TextureBuilder::Create(const TextureProperty& property, VkImage& image
	,VkDeviceMemory& memory, std::vector<VkImageView>& viewArray)
{
	if(image == VK_NULL_HANDLE && memory == VK_NULL_HANDLE)
	{
		createImage(property.image, image, memory);
	}

	for (int i = 0; i < property.viewArray.size(); i++)
	{
		//���̃e�N�X�`���̃r���[���쐬
		createImageView(property.viewArray[i], image, viewArray[i]);
	}
}