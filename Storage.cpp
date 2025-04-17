#include"Storage.h"
#include"VulkanBase.h"

Storage* Storage::storage = nullptr;

Storage::Storage()
{
	pointLightDescSet = nullptr;
	directionalLightDescSet = nullptr;
}

void Storage::cleanup()
{

}

//gltfモデルを読み込んだ際に、このクラスに格納する。
//再びそのgltfモデルが必要になった場合は、このクラスから参照取得する
void Storage::addModel(std::string obj, GltfModel* model)
{
	gltfModelStorage[obj] = std::shared_ptr<GltfModel>(model);
}

//上と同様、画像を読み込んだ際にこのクラスに格納する
void Storage::addImageData(std::string path,ImageData* image)
{
	imageDataStorage[path] = std::shared_ptr<ImageData>(image);
}

//VulkanBaseからカメラにアクセス出るように、Sceneクラスのカメラを設定する
void Storage::setCamera(std::shared_ptr<Camera> c)
{
	camera = c;
}

//Sceneクラスでluaで設定されたオブジェクトはこのクラスに格納し、このクラスからVulkanBaseでそのオブジェクトのレンダリングを行う
void Storage::addModel(std::shared_ptr<Model> model)
{
	VulkanBase::GetInstance()->setModelData(model);
	sceneModelStorage.push_back(std::shared_ptr<Model>(model));
}

//Modelクラス同様、ライトもこのクラスに格納し、レンダリング時にVulkanBaseから利用される
void Storage::addLight(std::shared_ptr<PointLight> pl)
{
	scenePointLightStorage.push_back(pl);
}

//Modelクラス同様、ライトもこのクラスに格納し、レンダリング時にVulkanBaseから利用される
void Storage::addLight(std::shared_ptr<DirectionalLight> dl)
{
	sceneDirectionalLightStorage.push_back(dl);
}

//UIの追加
void Storage::addUI(std::shared_ptr<UI> ui)
{
	VulkanBase::GetInstance()->setUI(ui);
	uiStorage.push_back(ui);
}

//通常のレンダリングで必要なdescriptorSetの作成
void Storage::prepareDescriptorSets()
{
	VulkanBase::GetInstance()->prepareDescriptorSets();
}

void Storage::prepareLightsForVulkan()
{
	VulkanBase::GetInstance()->setLightData(scenePointLightStorage, sceneDirectionalLightStorage);//ライト用のバッファの用意
	VulkanBase::GetInstance()->setCubeMapModel(cubemap);//キューブマップ用のバッファの用意
}

//descriptorSetの用意
void Storage::prepareDescriptorData()
{
	//DescriptorSetLayoutを用意する
	int lightCount = static_cast<int>(sceneDirectionalLightStorage.size()) + static_cast<int>(scenePointLightStorage.size());
	VulkanBase::GetInstance()->prepareDescriptorData(lightCount);

	for (auto gltfModel : gltfModelStorage)
	{
		//gltfMdodelクラスのテクスチャなどを用意する
		VulkanBase::GetInstance()->setGltfModelData(gltfModel.second);
	}
}

//カメラへの参照を返す
std::shared_ptr<Camera> Storage::accessCamera()
{
	return camera;
}

//求められたリソースがすでにこのクラスに格納されているかどうかを返す
//この関数では、その判定のみを担う
bool Storage::containModel(std::string path)
{
	if (gltfModelStorage[path] != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//求められたリソースがすでにこのクラスに格納されているかどうかを返す
//この関数では、その判定のみを担う
bool Storage::containImageData(std::string path)
{
	if (imageDataStorage[path] != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//このクラスにすでに格納されたgltfModelを返す
std::unordered_map<std::string, std::shared_ptr<GltfModel>>& Storage::getgltfModel()
{
	return gltfModelStorage;
}

//このクラスにすでに格納されたgltfModelのmapを返す
std::shared_ptr<GltfModel> Storage::getgltfModel(std::string obj)
{
	return gltfModelStorage[obj];
}

//このクラスにすでに格納された画像を返す
std::shared_ptr<ImageData> Storage::getImageData(std::string path)
{
	return imageDataStorage[path];
}

//各種ライト用のバッファを返す、なお種類ごとに複数のライトを一つの配列としてまとめて構造体にしているため
//同じ種類のライトがいくつあろうと、このバッファは一つのみ
MappedBuffer& Storage::getPointLightsBuffer()
{
	return pointLightsBuffer;
}

//各種ライト用のバッファを返す、なお種類ごとに複数のライトを一つの配列としてまとめて構造体にしているため
//同じ種類のライトがいくつあろうと、このバッファは一つのみ
MappedBuffer& Storage::getDirectionalLightsBuffer()
{
	return directionalLightsBuffer;
}

//コライダー用のAABBを計算する
void Storage::calcSceneBoundingBox(glm::vec3& boundingMin, glm::vec3& boundingMax)
{
	boundingMin = glm::vec3(FLT_MAX);
	boundingMax = glm::vec3(-FLT_MAX);

	for (auto model : storage->getModels())
	{
		glm::vec3 minimum = (model->scale * model->getGltfModel()->initPoseMin) + model->getPosition();
		glm::vec3 max = (model->scale * model->getGltfModel()->initPoseMax) + model->getPosition();

		if (boundingMin.x > minimum.x)
		{
			boundingMin.x = minimum.x;
		}
		if (boundingMin.y > minimum.y)
		{
			boundingMin.y = minimum.y;
		}
		if (boundingMin.z > minimum.z)
		{
			boundingMin.z = minimum.z;
		}

		if (boundingMax.x < max.x)
		{
			boundingMax.x = max.x;
		}
		if (boundingMax.y < max.y)
		{
			boundingMax.y = max.y;
		}
		if (boundingMax.z < max.z)
		{
			boundingMax.z = max.z;
		}
	}
}

//キューブマッピング用のHDRI画像を設定する
void Storage::setCubemapTexture(std::shared_ptr<ImageData> image)
{
	cubemapImage = image;
}

//HDRI画像を返す、キューブマップ作成時のVulkanBaseから呼び出される
std::shared_ptr<ImageData> Storage::getCubemapImage()
{
	return cubemapImage;
}