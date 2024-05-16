#include"VkStorage.h"

void VkStorage::setDescriptorSetLayout(VkModel* model)
{
    auto layoutBit = model->getLayoutBit();
    if (layoutStorage.contains(layoutBit))
    {
        model->setDescriptorSetLayout(layoutStorage[layoutBit].get());
        setPipeline(model);
        return;
    }

    VkDescriptorSetLayout* descriptorSetLayout = VulkanBase::GetInstance()->createDescriptorSetLayout(model);

    layoutStorage[layoutBit] = std::unique_ptr<VkDescriptorSetLayout>(descriptorSetLayout);

    model->setDescriptorSetLayout(layoutStorage[layoutBit].get());
}

void VkStorage::setPipeline(VkModel* model)
{
    auto layout = model->getLayoutBit();
    if (pipelineStorage.contains(layout))
    {
        model->setPipeline(pipelineStorage[layout].get());
        return;
    }

    VkPipeline* pipeline = VulkanBase::GetInstance()->createGraphicsPipeline();

    pipelineStorage[layout] = std::unique_ptr<VkPipeline>(pipeline);
    model->setPipeline(pipelineStorage[layout].get());
}

void VkStorage::setDescriptorPool(VkModel* model)
{
    auto bit = model->getLayoutBit();
    if (poolStorage.contains(bit))
    {
        model->setDescriptorPool(poolStorage[bit].get());
    }

    VkDescriptorPool* pool = VulkanBase::GetInstance()->createDescriptorPool(model);
    poolStorage[bit] = std::unique_ptr <VkDescriptorPool> (pool);
    model->setDescriptorPool(poolStorage[bit].get());
}

void VkStorage::setDescriptorSet(VkModel* model)
{
    auto pair = model->getPairLayoutImage();
    if (descriptorSetStorage.contains(pair))
    {
        model->setDescriptorSet(descriptorSetStorage[pair].get());
        return;
    }

    VkDescriptorSet* descriptor = VulkanBase::GetInstance()->createDescriptorSets(model);
    descriptorSetStorage[pair] = std::unique_ptr<VkDescriptorSet>(descriptor);
    model->setDescriptorSet(descriptorSetStorage[pair].get());
}