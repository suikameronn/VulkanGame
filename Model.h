#pragma once

#include<vector>
#include<bitset>
#include <time.h>

#include"EnumList.h"
#include"GltfModel.h"
#include"Colider.h"
#include"RTree.h"
#include"Light.h"

class Scene;

struct Ray
{
	float length;
	glm::vec3 origin;
	glm::vec3 direction;
};

//フレームバッファとしてのgpu上の画像用のバッファーの構造体
//その画像へのビューも持つ
struct FrameBufferAttachment {
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;

	void destory(VkDevice& device)
	{
		vkDestroyImageView(device, view, nullptr);
		vkDestroyImage(device, image, nullptr);
		vkFreeMemory(device, memory, nullptr);
	}
};

//シャドウマッピングやキューブマッピング用のオフスクリーンレンダリング用の構造体
struct OffScreenPass {
	int32_t width, height;//レンダリングの出力サイズ
	std::vector<VkFramebuffer> frameBuffer;//レンダリングの出力先のバッファー
	std::vector<FrameBufferAttachment> imageAttachment;//レンダリングの出力先を示すもの
	VkRenderPass renderPass;//利用するレンダーパス
	VkSampler sampler;//レンダリング結果へのサンプラー
	VkDescriptorSetLayout layout;//レンダリング用のレイアウト
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	std::vector<VkDescriptorSet> descriptorSets;

	OffScreenPass()
	{
		width = 0;
		height = 0;

		for (auto buffer : frameBuffer)
		{
			buffer = nullptr;
		}

		for (auto attachment : imageAttachment)
		{
			attachment.image = nullptr;
			attachment.memory = nullptr;
			attachment.view = nullptr;
		}

		renderPass = nullptr;
		sampler = nullptr;
		layout = nullptr;
		pipelineLayout = nullptr;
		pipeline = nullptr;

		for (auto descriptorSet : descriptorSets)
		{
			descriptorSet = nullptr;
		}
	}

	void setFrameCount(int count)//オフスクリーンレンダリングを行うフレーム数の設定
	{
		frameBuffer.resize(count);
		imageAttachment.resize(count);
		descriptorSets.resize(count);
	}

	void destroy(VkDevice& device)
	{
		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, layout, nullptr);

		for (auto& framebuffer : frameBuffer)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		for (auto& attachment : imageAttachment)
		{
			attachment.destory(device);
		}

		vkDestroySampler(device, sampler, nullptr);

		vkDestroyRenderPass(device, renderPass, nullptr);
	}
};

struct FragmentParam
{
	//透明度を上書き
	float alphaness;

	FragmentParam()
	{
		//あり得ない値にして
		//この値を使うか使わないかのフラグにする
		alphaness = -1.0f;
	}
};

//通常のレンダリング用のuniform buffer
struct MatricesUBO {
	glm::vec3 scale;//uv座標調整用
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(16) glm::vec4 worldCameraPos;
	alignas(16) int lightCount;
	alignas(16) std::array<glm::mat4, 20> lightMVP;//ライトの行列
};

//通常のレンダリングのアニメーション用の行列
struct AnimationUBO
{
	alignas(16) glm::mat4 nodeMatrix;
	alignas(16) glm::mat4 matrix;
	alignas(16) std::array<glm::mat4, 128> boneMatrix;
	alignas(16) int boneCount;
};

//シャドウマップ作成用のuniform buffer
struct ShadowMapUBO
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

//シャドウマップ作成用の構造体
struct ShadowMapData
{
	//平衡投影の行列
	glm::mat4 proj;

	//シャドウマップの解像度の倍率を設定
	int shadowMapScale;
	//オフスクリーンレンダリング用の構造体
	OffScreenPass passData;
	//シャドウマップ作成用の行列の配列
	std::vector<ShadowMapUBO> matUBOs;
	//行列用のバッファの配列
	std::vector<MappedBuffer> mappedBuffers;

	//シャドウマップを通常のレンダリングで使用するためのデータ
	std::vector<VkDescriptorSet> descriptorSets;

	//シーン上のライトの数だけ作成
	void setFrameCount(int frameCount)
	{
		matUBOs.resize(frameCount);
		mappedBuffers.resize(frameCount);
		passData.setFrameCount(frameCount);
	}

	void destroy(VkDevice& device)
	{
		passData.destroy(device);

		for (auto& buffer : mappedBuffers)
		{
			buffer.destroy(device);
		}
	}
};

//3Dモデルを持つオブジェクトを担うクラス
class Model:public Object
{
protected:

	//オブジェクトの中心位置
	glm::vec3 pivot;

	//半透明描画のフラッグ
	bool transparent;

	//R-tree用の初期のAABB
	glm::vec3 initMin, initMax;
	//AABB
	glm::vec3 min, max;

	//現在所属しているRTreeのノード
	RNode<Model>* rNode;
	//RTreeのノード内のインデックス
	int rNodeIndex;

	//R-tree用のMBRについて最大値と最小値
	glm::vec3 mbrMin, mbrMax;

	//3Dモデルの初期スケール
	glm::vec3 initScale;
	
	//レイキャスト時使用
	Scene* scene;

	//物理挙動用
	std::unique_ptr<PhysicBase> physicBase;

	//uvにスケールを加え、テクスチャの引き延ばしを防ぐ
	bool uvScale;

	//gltfモデルへの参照
	std::shared_ptr<GltfModel> gltfModel;

	//コライダーを持つかどうか
	bool hasColiderFlag;
	//当たり判定を解消するかどうか
	bool trigger;

	//mvp行列用のバッファー
	MappedBuffer modelViewMappedBuffer;
	//アニメーション用行列のバッファ
	std::vector<MappedBuffer> animationMappedBuffers;
	//アニメーション用行列の配列
	std::vector<std::array<glm::mat4, 128>> jointMatrices;

	//アニメーション開始時間
	clock_t startTime;
	//現在の時間
	clock_t currentTime;
	//アニメーションの再生時間
	double deltaTime;
	//アニメーションの切り替えフラッグ、アニメーション開始時間をリセット
	//同一アニメーションをループさせる際、アニメーションが終了したときもtrueに
	bool animationChange;

	//gltfモデルのマテリアルの上書き用
	std::shared_ptr<Material> material;
	//コライダー
	std::shared_ptr<Colider> colider;

	//アイドル時に再生するアニメーションの名前
	std::string defaultAnimationName;
	//現在再生しているアニメーションの名前
	std::string currentPlayAnimationName;
	//次の再生する予定のアニメーション
	std::string nextPlayAnimationName;
	//gltfモデルの持つアニメーションの名前の配列
	std::vector<std::string> animationNames;
	//ノードごとのトランスフォームを記録
	NodeTransform nodeTransform;

	//自身が床の場合、上に載っているオブジェクトの配列、自身の移動時、それらのオブジェクトも追従させる
	std::list<std::weak_ptr<Model>> groundingObjects;

	//コライダー用のAABBからMBRを計算
	void calcMBR();

	//フラグメントシェーダに渡すパラメータ
	FragmentParam fragParam;

	virtual void updateUniformBuffer(GltfNode* node);
	virtual void updateUniformBuffer(std::list<std::shared_ptr<DirectionalLight>>& dirLights
		, std::list<std::shared_ptr<PointLight>>& pointLights, ShadowMapData& shadowMapData);

	virtual void cleanupVulkan();//Vulkanの変数の破棄

public:

	Model();
	Model(std::string luaScriptPath);
	~Model()
	{
		if (rNode)
		{
			rNode->deleteObject(this);
		}

		if (lua)
		{
			lua_close(lua);
		}

		cleanupVulkan();
	}

	bool isTransparent()
	{
		return transparent;
	}

	void setTransparent(bool t)
	{
		transparent = t;
	}

	glm::vec3 getPivot()
	{
		return pivot;
	}

	void registerGlueFunctions() override;//glue関数の設定

	float gravity;//重力の強さ
	void setZeroVelocity();//速度のリセット
	void cancelGravity();//重力の打ち消し

	//RTree用のAABBを設定する
	void getMbrMinMax(glm::vec3& min, glm::vec3& max)
	{
		min = this->mbrMin;
		max = this->mbrMax;
	}

	glm::vec3 getMbrMin()
	{
		return mbrMin;
	}

	glm::vec3 getMbrMax()
	{
		return mbrMax;
	}

	FragmentParam& getFragmentParam() { return fragParam; }

	//初期スケールを設定
	void setInitScale(glm::vec3 s) { initScale = s; }

	glm::vec3 getScale() { return initScale * scale; }

	//現在所属しているノードを設定する
	void setRNode(RNode<Model>* node,int index) 
	{ 
		rNode = node; 
		rNodeIndex = index;
	}

	int getRNodeIndex()
	{
		return rNodeIndex;
	}

	//スケール
	glm::vec3 scale;

	//自分が床の上に立っているかどうか、立っている場合は重力の影響を無視する
	bool isGrounding;

	void setDefaultAnimationName(std::string name);//デフォルトのアニメーションを設定

	void sendPosToChildren();//子オブジェクトに親の移動を反映
	void sendPosToCamera(glm::vec3 targetPos);//カメラに指定した位置を追従させる

	void setUVScale() { uvScale = true; }//uvにスケールを加え、テクスチャの引き延ばしを防ぐよう設定する
	bool applyScaleUV() { return uvScale; }

	void setgltfModel(std::shared_ptr<GltfModel> model);//3DCGモデルを登録
	GltfNode* getRootNode() { return gltfModel->getRootNode(); }
	std::shared_ptr<GltfModel> getGltfModel() { return gltfModel; }

	void setBaseColor(glm::vec4 baseColor);//モデルの色を上書き

	void switchPlayAnimation();//アニメーションをデフォルトに切り替える
	void switchPlayAnimation(std::string nextAnimation);//指定のアニメーションに切り替える
	void playAnimation();//アニメーションの再生
	std::array<glm::mat4, 128>& getJointMatrices(int index);//特定のスケルトンのアニメーション行列を計算

	std::vector<DescSetData> descSetDatas;
	BufferObject* getPointBufferData() { return gltfModel->getPointBuffer(); }//頂点用バッファの取得
	MappedBuffer& getModelViewMappedBuffer() { return modelViewMappedBuffer; }//モデルビュー行列用のバッファの取得
	MappedBuffer* getAnimationMappedBufferData();//アニメーション用の行列のバッファの取得

	//衝突を解消時、このオブジェクトを動かせるかどうか、障害物の場合、このオブジェクトは動かさないため,falseとなる
	bool isMovable;
	//コライダーを持っているかどうか
	bool hasColider();
	void setColider(bool isTrigger);//コライダーの設定
	std::shared_ptr<Colider> getColider() { return colider; }
	
	std::weak_ptr<Model> rayCast(glm::vec3 origin,glm::vec3 dir,float maxLength,glm::vec3& normal);

	void updateTransformMatrix() override;//座標変換行列の更新

	void setPosition(glm::vec3 pos) override;//位置の設定

	void Update() override;//更新処理
	void customUpdate() override;//特殊な更新処理

	glm::vec3 getLastScale();
	void setLastFrameTransform() override;//一つ前のフレームの座標などのデータを設定

	void createTransformTable() override;

	void sendTransformToLua() override;//luaに座標などを送る
	void receiveTransformFromLua() override;//luaから座標などを受け取る

	void initFrameSetting() override;//初期フレームのみの処理

	bool isGround(glm::vec3& normal);//オブジェクトが床に接しているかどうか
	void addGroundingObject(std::weak_ptr<Model> object);//床に接していたらそれを追加
	void clearGroundingObject();

	virtual void frameEnd(std::list<std::shared_ptr<DirectionalLight>>& dirLights
		, std::list<std::shared_ptr<PointLight>>& pointLights, ShadowMapData& shadowMapData);

	//衝突時の処理(特に何もしない)
	virtual void collision(std::shared_ptr<Model> model) {};
};

/*以下の静的関数はluaスクリプト上から呼び出される*/

namespace glueModelFunction//Modelクラス用のglue関数
{
	//Modelクラスのスケールの設定
	static int glueSetScale(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -1));

		switch (obj->getObjNum())
		{
		case 1:
			Model* model = dynamic_cast<Model*>(obj);
			model->scale.x = static_cast<float>(lua_tonumber(lua, -4));
			model->scale.y = static_cast<float>(lua_tonumber(lua, -3));
			model->scale.z = static_cast<float>(lua_tonumber(lua, -2));
			break;
		}

		return 0;
	}
}