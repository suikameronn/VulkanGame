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

    backGroundColor.destroy(device);

    //IBLのデータの破棄
    iblDiffuse.destroy(device);
    iblSpecularReflection.destroy(device);
    iblSpecularBRDF.destroy(device);

	for (size_t i = 0; i < pointBuffers.size(); i++)//頂点用バッファの解放
	{
		vkDestroyBuffer(device, pointBuffers[i].vertBuffer, nullptr);
		vkFreeMemory(device, pointBuffers[i].vertHandler, nullptr);

		vkDestroyBuffer(device, pointBuffers[i].indeBuffer, nullptr);
		vkFreeMemory(device, pointBuffers[i].indeHandler, nullptr);
	}

	//uniform bufferの解放
	vkDestroyBuffer(device, modelViewMappedBuffer.uniformBuffer, nullptr);
	vkFreeMemory(device, modelViewMappedBuffer.uniformBufferMemory, nullptr);
	modelViewMappedBuffer.uniformBufferMapped = nullptr;

	//アニメーションのユニフォームバッファの解放
	for (int i = 0; i < (int)animationMappedBuffers.size(); i++)
	{
		vkDestroyBuffer(device, animationMappedBuffers[i].uniformBuffer, nullptr);
		vkFreeMemory(device, animationMappedBuffers[i].uniformBufferMemory, nullptr);
		animationMappedBuffers[i].uniformBufferMapped = nullptr;
	}
}