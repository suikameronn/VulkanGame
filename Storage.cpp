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