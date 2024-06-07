#include"Storage.h"
#include"VulkanBase.h"

Storage* Storage::storage = nullptr;

void Storage::cleanup()
{
	for (auto itr = descriptorStorage.begin(); itr != descriptorStorage.end(); itr++)
	{

	}
}

//StorageにFbxModelを追加する
void Storage::addModel(OBJECT obj, FbxModel* model)
{
	fbxModelStorage[obj] = std::shared_ptr<FbxModel>(model);
}

//StorageにDescriptorInfoを追加する
void Storage::addDescriptorInfo(std::bitset<8> layoutBit, DescriptorInfo* info)
{
	descriptorStorage[layoutBit] = info;
}

//StorageにCameraを追加する
void Storage::setCamera(Camera* c)
{
	camera = c;
}

//StorageにModelを追加する
void Storage::addModel(Model* model)
{
	model->datasResize();

	VulkanBase::GetInstance()->setModelData(model);
	sceneModelStorage[model->getDescriptorInfo(0)].push_back(std::move(std::unique_ptr<Model>(model)));
}

//Storageから指定されたDescriptorInfoへの参照を返す
DescriptorInfo* Storage::accessDescriptorInfo(std::bitset<8> layoutBit)
{
	return descriptorStorage[layoutBit];
}

//StorageからDescriptorInfoのマップへの参照を返す
void Storage::accessDescriptorInfoItr(std::unordered_map<std::bitset<8>, DescriptorInfo*>::iterator& begin,
	std::unordered_map<std::bitset<8>, DescriptorInfo*>::iterator& end)
{
	begin = descriptorStorage.begin();
	end = descriptorStorage.end();
}

//StorageのCameraにアクセスする
Camera* Storage::accessCamera()
{
	return camera;
}

//Storageから指定された個別のグループのvectorの参照を返す
void Storage::accessModelVector(std::unordered_map<DescriptorInfo*, std::vector<std::unique_ptr<Model>>, Hash>::iterator current,
	std::vector<std::unique_ptr<Model>>::iterator& itr,std::vector<std::unique_ptr<Model>>::iterator& itr2)
{
	itr = current->second.begin();
	itr2 = current->second.end();
}

//Storageから指定された全体のunordered_mapの参照を返す
void Storage::accessModelUnMap(std::unordered_map<DescriptorInfo*, std::vector<std::unique_ptr<Model>>, Hash>::iterator* itr,
	std::unordered_map<DescriptorInfo*, std::vector<std::unique_ptr<Model>>, Hash>::iterator* itr2)
{
	*itr = sceneModelStorage.begin();
	*itr2 = sceneModelStorage.end();
}

//Storageに指定されあFbxModelが既に存在しているかどうかを返す
bool Storage::containModel(OBJECT obj)
{
	if (fbxModelStorage[obj] != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//Storageに指定されたDescritporInfoがすでに存在するかどうかを返す
bool Storage::containDescriptorInfo(std::bitset<8> layoutBit)
{
	if (descriptorStorage[layoutBit] != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//StorageからFbxModelを読み取る
std::shared_ptr<FbxModel> Storage::getFbxModel(OBJECT obj)
{
	return fbxModelStorage[obj];
}