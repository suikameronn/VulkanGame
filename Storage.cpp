#include"Storage.h"
#include"VulkanBase.h"

Storage* Storage::storage = nullptr;

Storage::Storage()
{
}

void Storage::cleanup()
{

}

//gltfモデルを読み込んだ際に、このクラスに格納する。
//再びそのgltfモデルが必要になった場合は、このクラスから参照取得する
void Storage::addModel(std::string obj, std::shared_ptr<GltfModel> model)
{
	std::lock_guard<std::mutex> lock(mtx);

	gltfModelStorage[obj] = model;
}

//上と同様、画像を読み込んだ際にこのクラスに格納する
void Storage::addImageData(std::string path,ImageData* image)
{
	std::lock_guard<std::mutex> lock(mtx);

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

//このクラスにすでに格納されたimageDataのmapを返す
std::unordered_map<std::string, std::shared_ptr<ImageData>>& Storage::getImageData()
{
	return imageDataStorage;
}

//このクラスにすでに格納されたgltfModelのmapを返す
std::shared_ptr<GltfModel> Storage::getgltfModel(std::string obj)
{
	if (!containModel(obj))
	{
		std::cerr << "Not register GltfModel" << std::endl;
	}

	return gltfModelStorage[obj];
}

//このクラスにすでに格納された画像を返す
std::shared_ptr<ImageData> Storage::getImageData(std::string path)
{
	if (!containImageData(path))
	{
		std::cerr << "Not register Image" << std::endl;
	}

	return imageDataStorage[path];
}