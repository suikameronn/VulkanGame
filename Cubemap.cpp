#include"Storage.h"
#include"VulkanBase.h"

#include"Cubemap.h"

void Cubemap::updateUniformBuffer()
{
    Storage* storage = Storage::GetInstance();
    std::shared_ptr<Camera> camera = storage->accessCamera();

    MatricesUBO ubo{};

    ubo.view = camera->cubemapViewMat;
    ubo.proj = camera->perspectiveMat;

    memcpy(modelViewMappedBuffer.uniformBufferMapped, &ubo, sizeof(ubo));
}

void Cubemap::frameEnd()
{
    updateUniformBuffer();
}

void Cubemap::cleanupVulkan()
{
    VkDevice device = VulkanBase::GetInstance()->getDevice();
               
    iblDiffuse.destroy(device);
    iblSpecularReflection.destroy(device);
    iblSpecularBRDF.destroy(device);
}