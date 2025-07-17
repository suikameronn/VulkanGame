#pragma once

#include"GltfModelComponent.h"

#include"GpuBufferFactory.h"
#include"DescriptorSetFactory.h"


//ノードごとのトランスフォームを記録
struct NodeTransform
{
	std::vector<glm::vec3> translation;
	std::vector<glm::quat> rotation;
	std::vector<glm::vec3> scale;

	std::vector<glm::mat4> matrix;
	std::vector<glm::mat4> nodeTransform;

	void setNodeCount(int nodeCount)
	{
		translation.resize(nodeCount);
		rotation.resize(nodeCount);
		scale.resize(nodeCount);
		matrix.resize(nodeCount);
		nodeTransform.resize(nodeCount);

		std::fill(translation.begin(), translation.end(), glm::vec3(0.0f));
		std::fill(rotation.begin(), rotation.end(), glm::quat());
		std::fill(scale.begin(), scale.end(), glm::vec3(1.0f));
		std::fill(matrix.begin(), matrix.end(), glm::mat4(1.0f));
		std::fill(nodeTransform.begin(), nodeTransform.end(), glm::mat4(1.0f));
	}
};

//通常のレンダリングのアニメーション用の行列
struct AnimationUBO
{
	alignas(16) glm::mat4 nodeMatrix;
	alignas(16) glm::mat4 matrix;
	alignas(16) std::array<glm::mat4, 128> boneMatrix;
	alignas(16) int boneCount;
};

class AnimationComponent : public IComponent
{
private:

	//バッファの作成
	std::shared_ptr<GpuBufferFactory> bufferFactory;
	//ディスクリプタセットレイアウトの作成
	std::shared_ptr<GpuDescriptorSetLayoutFactory> layoutFactory;
	//ディスクリプタセットの作成
	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;

	//プリミティブごとのDescriptorSet
	std::vector<std::shared_ptr<DescriptorSet>> descriptorSet;

	//Gltfモデルのデータ
	std::shared_ptr<GltfModel> gltfModel;

	//ノードごとのトランスフォーム
	NodeTransform nodeTransform;

	//経過時間
	double deltaTime;
	//アニメーションを再生し始めた時間
	double startTime;

	//アニメーションの切り替えフラッグ、アニメーション開始時間をリセット
	//同一アニメーションをループさせる際、アニメーションが終了したときもtrueに
	bool animationChange;

	//現在再生しているアニメーションの名前
	std::string currentPlayAnimationName;
	//次の再生する予定のアニメーション
	std::string nextPlayAnimationName;
	//gltfモデルの持つアニメーションの名前の配列
	std::vector<std::string> animationNames;

	//アニメーション用行列の配列
	std::vector<std::array<glm::mat4, 128>> jointMatrices;

	//アニメーション用行列を記録するためのバッファ
	std::vector<std::shared_ptr<GpuBuffer>> animationBuffer;

	//アニメーション用行列を結び付けるディスクリプタセット
	std::vector<std::shared_ptr<DescriptorSet>> descriptorSet;

	//GPU上のバッファを作成する
	void createBuffer();
	void createBuffer(const GltfNode* node);

	//ディスクリプタセットを作る
	void createDescriptorSet();
	void createDescriptorSet(const GltfNode* node);

public:

	AnimationComponent(std::shared_ptr<GpuBufferFactory> buffer
		, std::shared_ptr<GpuDescriptorSetLayoutFactory> layout
		, std::shared_ptr<DescriptorSetFactory> desc);

	//3Dモデルを設定する
	void setGltfModel(std::shared_ptr<GltfModel> model);

	//アニメーションを切り替える
	void switchPlayAnimation(std::string next);

	////特定のスケルトンのアニメーション行列を取得
	std::shared_ptr<DescriptorSet> getJointMatrices(int index);

	//コンポーネントをアタッチした時点で実行
	void OnAwake() override {};

	//アタッチ後の次のフレーム開始時点で実行
	void OnStart() override;

	//更新フェーズで実行
	void OnUpdate() override {};

	//更新フェーズ後に実行
	void OnLateUpdate() override {};

	//フレーム終了時に実行
	void OnFrameEnd() override;
};