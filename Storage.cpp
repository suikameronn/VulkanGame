#include"Storage.h"
#include"VulkanBase.h"

Storage* Storage::storage = nullptr;

void Storage::cleanup()
{
	for (auto itr = descriptorStorage.begin(); itr != descriptorStorage.end(); itr++)
	{

	}
}

//Storage��GltfModel��ǉ�����
void Storage::addModel(OBJECT obj, GltfModel* model)
{
	fbxModelStorage[obj] = std::shared_ptr<GltfModel>(model);
}

void Storage::addAnimation(OBJECT obj, Animation* animation)
{
	fbxAnimationStorage[obj] = std::shared_ptr<Animation>(animation);
}

void Storage::addImageData(std::string path,ImageData* image)
{
	imageDataStorage[path] = std::shared_ptr<ImageData>(image);
}

//Storage��DescriptorInfo��ǉ�����
void Storage::addDescriptorInfo(PrimitiveTextureCount ptc, DescriptorInfo& info)
{
	descriptorStorage[ptc] = info;
}

//Storage��Camera��ǉ�����
void Storage::setCamera(std::shared_ptr<Camera> c)
{
	camera = c;
}

//Storage��Model��ǉ�����
void Storage::addModel(std::shared_ptr<Model> model)
{
	DescriptorInfo info{};
	VulkanBase::GetInstance()->setModelData(model);
	sceneModelStorage.push_back(model);
}

//Storage����w�肳�ꂽDescriptorInfo�ւ̎Q�Ƃ�Ԃ�
DescriptorInfo* Storage::accessDescriptorInfo(PrimitiveTextureCount ptc)
{
	return &descriptorStorage[ptc];
}

//Storage����DescriptorInfo�̃}�b�v�ւ̎Q�Ƃ�Ԃ�
void Storage::accessDescriptorInfoItr(std::unordered_map<PrimitiveTextureCount, DescriptorInfo>::iterator& begin,
	std::unordered_map<PrimitiveTextureCount, DescriptorInfo>::iterator& end)
{
	begin = descriptorStorage.begin();
	end = descriptorStorage.end();
}

void Storage::accessFbxModel(std::unordered_map<OBJECT, std::shared_ptr<GltfModel>>::iterator& itr,
	std::unordered_map<OBJECT, std::shared_ptr<GltfModel>>::iterator& itr2)
{
	itr = fbxModelStorage.begin();
	itr2 = fbxModelStorage.end();
}

//Storage��Camera�ɃA�N�Z�X����
std::shared_ptr<Camera> Storage::accessCamera()
{
	return camera;
}

//Storage�Ɏw�肳�ꂠGltfModel�����ɑ��݂��Ă��邩�ǂ�����Ԃ�
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

bool Storage::containAnimation(OBJECT obj)
{
	if (fbxAnimationStorage[obj] != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

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

//Storage�Ɏw�肳�ꂽDescritporInfo�����łɑ��݂��邩�ǂ�����Ԃ�
bool Storage::containDescriptorInfo(PrimitiveTextureCount ptc)
{
	if (descriptorStorage.find(ptc) != descriptorStorage.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

//Storage����GltfModel��ǂݎ��
std::shared_ptr<GltfModel> Storage::getFbxModel(OBJECT obj)
{
	return fbxModelStorage[obj];
}

std::shared_ptr<Animation> Storage::getAnimation(OBJECT obj)
{
	return fbxAnimationStorage[obj];
}

std::shared_ptr<ImageData> Storage::getImageData(std::string path)
{
	return imageDataStorage[path];
}