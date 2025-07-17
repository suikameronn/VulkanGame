#pragma once

#include"GltfModel.h"

#include"IComponent.h"

#include"GpuBufferFactory.h"
#include"DescriptorSetFactory.h"

class GltfModelComponent : public IComponent
{
private:

	//バッファの作成
	std::shared_ptr<GpuBufferFactory> bufferFactory;
	//ディスクリプタセットレイアウトの作成
	std::shared_ptr<GpuDescriptorSetLayoutFactory> layoutFactory;
	//ディスクリプタセットの作成
	std::shared_ptr<DescriptorSet> descriptorSetFactory;

	//Gltfモデルのデータ
	std::shared_ptr<GltfModel> gltfModel;

public:

	GltfModelComponent(std::shared_ptr<GpuBufferFactory> buffer
		, std::shared_ptr<GpuDescriptorSetLayoutFactory> layout
		, std::shared_ptr<DescriptorSet> desc
		, std::shared_ptr<GltfModel> model);

	//Gltfモデルを返す
	std::shared_ptr<GltfModel> getGltfModel() { return gltfModel; }

	//3Dモデルの中心を求める
	glm::vec3 getCenter();

	//コンポーネントをアタッチした時点で実行
	void OnAwake() override;

	//アタッチ後の次のフレーム開始時点で実行
	void OnStart() override {};

	//更新フェーズで実行
	void OnUpdate() override {};

	//更新フェーズ後に実行
	void OnLateUpdate() override {};

	//フレーム終了時に実行
	void OnFrameEnd() override {};
};