#pragma once

#include"IComponent.h"

#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include"GpuBufferFactory.h"
#include"DescriptorSetFactory.h"

struct Transform
{
	alignas(16) glm::vec3 uvScale;
	glm::mat4 modelMatrix;
};

class TransformComponent : public IComponent
{
private:

	Transform transform;

	//GPUのバッファを作成する
	std::shared_ptr<GpuBufferFactory> bufferFactory;

	//ディスクリプタセットのレイアウトを作成する
	std::shared_ptr<GpuDescriptorSetLayoutFactory> layoutFactory;

	//バッファを参照するためのディスクリプタを作成する
	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;

	//オブジェクトの中心
	glm::vec3 pivot;

	//モデル行列とビュー行列を掛け合わせた行列を記録したバッファ
	std::shared_ptr<GpuBuffer> matBuffer;

	//行列の記録されたバッファを結び付ける
	std::shared_ptr<DescriptorSet> matBufferBindDesc;

	//行列を記録するユニフォームバッファを作成する
	void createUniformBuffer();

	//ユニフォームバッファを示すディスクリプタセットを作成する
	void createDescriptorSet();

public:

	glm::vec3 position;
	glm::vec3 rotateDeg;
	glm::vec3 scale;

	glm::vec3 getPivot() { return pivot; }

	TransformComponent(std::shared_ptr<GpuBufferFactory> buffer
		,std::shared_ptr<DescriptorSetFactory> desc
		, std::shared_ptr<GpuDescriptorSetLayoutFactory> layout);

	//平行移動行列
	glm::mat4 tlanslateMat();

	//回転行列
	glm::mat4 rotateMat();

	//拡大行列
	glm::mat4 scaleMat();

	//TRS行列
	glm::mat4 transformMat();

	std::shared_ptr<GpuBuffer> getMatrixBuffer() { return matBuffer; }

	//コンポーネントをアタッチした時点で実行
	void OnAwake() override;

	//アタッチ後の次のフレーム開始時点で実行
	void OnStart() override {};

	//更新フェーズで実行
	void OnUpdate() override {};

	//更新フェーズ後に実行
	void OnLateUpdate() override {};

	//フレーム終了時に実行
	void OnFrameEnd() override;
};