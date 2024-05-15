#include"VkStorage.h"

void VkStorage::setDescriptorSetLayout(VkModel* model)
{
    auto layoutBit = model->getLayoutBit();
    if (layoutStorage.contains(layoutBit))
    {
        model->setDescriptorSetLayout(layoutStorage[layoutBit].get());
    }

    std::vector<VkDescriptorSetLayoutBinding> bindings;

    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings.push_back(uboLayoutBinding);

    ImageData* image = model->getImageData();
    if (image)
    { 
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings.push_back(samplerLayoutBinding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();

    VkDescriptorSetLayout* descriptorSetLayout = nullptr;
    if (vkCreateDescriptorSetLayout(VulkanBase::GetInstance()->getDevice(), &layoutInfo, nullptr, descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    layoutStorage[layoutBit] = std::unique_ptr<VkDescriptorSetLayout>(descriptorSetLayout);

    model->setDescriptorSetLayout(layoutStorage[layoutBit].get());
}

void VkStorage::setDescriptorSet(VkModel* model)
{
    auto pair = model->getPairLayoutImage();
    if (descriptorSetStorage.contains(pair))
    {
        model->setDescriptorSet(descriptorSetStorage[pair].get());
    }

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    int j;
    for (j = 0; j < vkModelData.size(); j++)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = vkModelData[j].texture.view;
        imageInfo.sampler = vkModelData[j].texture.sampler;

        descriptorWrites[j + 1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[j + 1].dstSet = descriptorSets[i];
        descriptorWrites[j + 1].dstBinding = 1;
        descriptorWrites[j + 1].dstArrayElement = 0;
        descriptorWrites[j + 1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[j + 1].descriptorCount = 1;
        descriptorWrites[j + 1].pImageInfo = &imageInfo;
    }

    vkUpdateDescriptorSets(VulkanBase::GetInstance()->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}