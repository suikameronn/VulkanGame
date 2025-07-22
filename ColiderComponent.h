#pragma once

#include"IComponent.h"
#include"GltfModelComponent.h"
#include"TransformComponent.h"

#include"GpuBufferFactory.h"
#include"DescriptorSetFactory.h"

#include"Colider.h"

class ColiderComponent : public IComponent
{
private:

	std::shared_ptr<TransformComponent> transformComponent;

	std::shared_ptr<Colider> colider;

	std::shared_ptr<GpuBufferFactory> bufferFactory;
	std::shared_ptr<GpuDescriptorSetLayoutFactory> layoutFactory;
	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;

public:

	ColiderComponent(std::shared_ptr<GpuBufferFactory> buffer
		, std::shared_ptr<GpuDescriptorSetLayoutFactory> layout
		, std::shared_ptr<DescriptorSetFactory> desc)
	{
		bufferFactory = buffer;
		layoutFactory = layout;
		descriptorSetFactory = desc;
	}

	//すべてのコンポーネントをアタッチしたときに実行
	void initilize(std::shared_ptr<TransformComponent> transform
		,std::shared_ptr<GltfModelComponent> gltfModel);

	std::shared_ptr<Colider> getColider()
	{
		return colider;
	}

	//初期フレームに実行
	void OnStart() override;

	//更新フェーズで実行
	void OnUpdate() override;

	//更新フェーズ後に実行
	void OnLateUpdate() override;
};
};