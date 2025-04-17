#pragma once
#include<bitset>
#include<unordered_map>

#include"EnumList.h"
#include"Light.h"
#include"Camera.h"
#include"Model.h"
#include"UI.h"

enum IMAGE
{
	IMAGETEST = 0,
};

class ImageData;

//読み込んだ画像やgltfモデルなどを記録しておくクラス
//画像やgltfモデルなどのリソースが求められた場合、まずこのクラスにすでに格納されていないか確認をする
class Storage
{
private:
	//gltfモデルが格納される
	std::unordered_map<std::string, std::shared_ptr<GltfModel>> gltfModelStorage;
	//ファイル名をキーとして、画像データを格納する
	std::unordered_map<std::string, std::shared_ptr<ImageData>> imageDataStorage;

	//VulkanBaseからアクセスできるようにSceneクラスのカメラへの参照を持たせる、以下同様
	std::shared_ptr<Camera> camera;
	//キューブマッピング用立方体
	std::shared_ptr<Model> cubemap;
	//キューブマッピング用HDRI画像
	std::shared_ptr<ImageData> cubemapImage;
	
	//ロード画面用のUI
	std::shared_ptr<UI> loadUI;

	//画面上に現れるオブジェクトを格納する
	std::vector<std::shared_ptr<Model>> sceneModelStorage;
	//シーン内のポイントライトを格納する
	std::vector<std::shared_ptr<PointLight>> scenePointLightStorage;
	//シーン内の平行光源を格納する
	std::vector<std::shared_ptr<DirectionalLight>> sceneDirectionalLightStorage;
	//UIを記録する
	std::vector<std::shared_ptr<UI>> uiStorage;

	//ポイントライト用のDescriptorSet 色と座標を持つuniform buffer
	VkDescriptorSet pointLightDescSet;
	//平行光源用のDescriptorSet 色と方向を持つuniform buffer
	VkDescriptorSet directionalLightDescSet;

	//ライトのデータは種類ごとに一つのバッファー上に配列として格納する
	//ポイントライトのuniform bufferを格納するためのバッファー
	MappedBuffer pointLightsBuffer;
	//平行光源ライトのuniform bufferを格納するためのバッファー
	MappedBuffer directionalLightsBuffer;

	Storage();
	~Storage()
	{
		cleanup();
	}
	static Storage* storage;

	void cleanup();
public:

	static Storage* GetInstance()
	{
		if (!storage)
		{
			storage = new Storage();
		}

		return storage;
	}

	//ロードUIの設定
	void setLoadUI(std::shared_ptr<UI> ui) { loadUI = ui; }
	//ロードUIを返す
	std::shared_ptr<UI>getLoadUI() { return loadUI; }
	
	//キューブマッピング用のHDRI画像を設定する
	void setCubemapTexture(std::shared_ptr<ImageData> image);
	//HDRI画像を返す、キューブマップ作成時のVulkanBaseから呼び出される
	std::shared_ptr<ImageData> getCubemapImage();
	//キューブマッピング用の立方体を返す、キューブマップ作成時のVulkanBaseから呼び出される
	std::shared_ptr<Model> getCubeMap() { return cubemap; }
	//画面上のオブジェクトを返す、レンダリング時にVulkanBaseから呼び出される
	std::vector<std::shared_ptr<Model>>& getModels() { return sceneModelStorage; }
	//ポイントライトを返す、レンダリング時にVulkanBaseから呼び出される
	std::vector<std::shared_ptr<PointLight>>& getPointLights() { return scenePointLightStorage; }
	//平行光源を返す、レンダリング時にVulkanBaseから呼び出される
	std::vector<std::shared_ptr<DirectionalLight>>& getDirectionalLights() { return sceneDirectionalLightStorage; }
	//ポイントライトと平行光源の数を取得する
	int getLightCount() { return static_cast<int>(scenePointLightStorage.size() + sceneDirectionalLightStorage.size()); }
	//UIの配列を返す
	std::vector<std::shared_ptr<UI>>& getUI() {return uiStorage; }

	//ポイントライトと平行光源のdescriptorSetを返す。ライトは配列としてまとめてgpuに渡すため、descriptorSetは種類ごとに一つ
	VkDescriptorSet& getPointLightDescriptorSet() { return pointLightDescSet; }
	VkDescriptorSet& getDirectionalLightDescriptorSet() { return directionalLightDescSet; }

	//キューブマッピング用の立方体を設定
	void setCubeMapModel(std::shared_ptr<Model> cube) { cubemap = cube; }

	//gltfモデルを読み込んだ際に、このクラスに格納する。
	//再びそのgltfモデルが必要になった場合は、このクラスから参照取得する
	void addModel(std::string obj, GltfModel* geo);
	//上と同様、画像を読み込んだ際にこのクラスに格納する
	void addImageData(std::string, ImageData* image);

	//VulkanBaseからカメラにアクセス出るように、Sceneクラスのカメラを設定する
	void setCamera(std::shared_ptr<Camera> c);

	//Sceneクラスでluaで設定されたオブジェクトはこのクラスに格納し、このクラスからVulkanBaseでそのオブジェクトのレンダリングを行う
	void addModel(std::shared_ptr<Model> model);
	//Modelクラス同様、ライトもこのクラスに格納し、レンダリング時にVulkanBaseから利用される
	void addLight(std::shared_ptr<PointLight> light);
	void addLight(std::shared_ptr<DirectionalLight> light);
	//UIの追加
	void addUI(std::shared_ptr<UI> ui);

	void prepareDescriptorSets();//通常のレンダリングで必要なdescriptorSetの作成
	void prepareDescriptorData();//ライトのバッファの用意
	void prepareLightsForVulkan();//descriptorSetの用意

	//このクラスにすでに格納されたgltfModelを返す
	std::shared_ptr<GltfModel> getgltfModel(std::string obj);
	//このクラスにすでに格納された画像を返す
	std::shared_ptr<ImageData> getImageData(std::string path);
	//このクラスにすでに格納されたgltfModelのmapを返す
	std::unordered_map<std::string, std::shared_ptr<GltfModel>>& getgltfModel();
	
	//各種ライト用のバッファを返す、なお種類ごとに複数のライトを一つの配列としてまとめて構造体にしているため
	//同じ種類のライトがいくつあろうと、このバッファは一つのみ
	MappedBuffer& getPointLightsBuffer();
	MappedBuffer& getDirectionalLightsBuffer();

	//カメラへの参照を返す
	std::shared_ptr<Camera> accessCamera();

	//コライダー用のAABBを計算する
	void calcSceneBoundingBox(glm::vec3& boundingMin, glm::vec3& boundingMax);

	//求められたリソースがすでにこのクラスに格納されているかどうかを返す
	//この関数では、その判定のみを担う
	bool containModel(std::string obj);
	bool containImageData(std::string path);

	//デストラクタ
	static void FinishStorage()
	{
		if (storage)
		{
			delete storage;
		}
	}
};