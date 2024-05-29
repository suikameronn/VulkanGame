#include"Storage.h"
#include"VulkanBase.h"

Storage* Storage::storage = nullptr;

void Storage::cleanup()
{
	for (auto itr = descriptorStorage.begin(); itr != descriptorStorage.end(); itr++)
	{

	}
}

//Storage��Meshes��ǉ�����
void Storage::addObj(OBJECT obj, Meshes* meshes)
{
	meshesStorage[obj].reset(meshes);
}

//Storage��ImageData��ǉ�����
void Storage::addImage(IMAGE image, ImageData* imageData)
{
	imageStorage[image].reset(imageData);
}

//Storage��DescriptorInfo��ǉ�����
void Storage::addDescriptorInfo(std::bitset<8> layoutBit, DescriptorInfo* info)
{
	descriptorStorage[layoutBit] = info;
}

//Storage��Camera��ǉ�����
void Storage::setCamera(Camera* c)
{
	camera = c;
}

//Storage��Model��ǉ�����
void Storage::addModel(Model* model)
{
	VulkanBase::GetInstance()->setModelData(model);
	sceneModelStorage[model->getDescriptorInfo()].push_back(std::move(std::unique_ptr<Model>(model)));
}

//Storage����w�肳�ꂽMeshes�ւ̎Q�Ƃ�Ԃ�
Meshes* Storage::accessObj(OBJECT obj)
{
	return meshesStorage[obj].get();
}

//Storage����w�肳�ꂽImageData�ւ̎Q�Ƃ�Ԃ�
ImageData* Storage::accessImage(IMAGE image)
{
	return imageStorage[image].get();
}

//Storage����w�肳�ꂽDescriptorInfo�ւ̎Q�Ƃ�Ԃ�
DescriptorInfo* Storage::accessDescriptorInfo(std::bitset<8> layoutBit)
{
	return descriptorStorage[layoutBit];
}

//Storage����DescriptorInfo�̃}�b�v�ւ̎Q�Ƃ�Ԃ�
void Storage::accessDescriptorInfoItr(std::unordered_map<std::bitset<8>, DescriptorInfo*>::iterator& begin,
	std::unordered_map<std::bitset<8>, DescriptorInfo*>::iterator& end)
{
	begin = descriptorStorage.begin();
	end = descriptorStorage.end();
}

//Storage��Camera�ɃA�N�Z�X����
Camera* Storage::accessCamera()
{
	return camera;
}

//Storage����w�肳�ꂽ�ʂ̃O���[�v��vector�̎Q�Ƃ�Ԃ�
void Storage::accessModelVector(std::unordered_map<DescriptorInfo*, std::vector<std::unique_ptr<Model>>, Hash>::iterator current,
	std::vector<std::unique_ptr<Model>>::iterator& itr,std::vector<std::unique_ptr<Model>>::iterator& itr2)
{
	itr = current->second.begin();
	itr2 = current->second.end();
}

//Storage����w�肳�ꂽ�S�̂�unordered_map�̎Q�Ƃ�Ԃ�
void Storage::accessModelUnMap(std::unordered_map<DescriptorInfo*, std::vector<std::unique_ptr<Model>>, Hash>::iterator* itr,
	std::unordered_map<DescriptorInfo*, std::vector<std::unique_ptr<Model>>, Hash>::iterator* itr2)
{
	*itr = sceneModelStorage.begin();
	*itr2 = sceneModelStorage.end();
}

//Storage�Ɏw�肳�ꂽMeshes�����łɑ��݂��邩�ǂ�����Ԃ�
bool Storage::containMeshes(OBJECT obj)
{
	if (meshesStorage[obj] != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//Storage�Ɏw�肳�ꂽImageData�����łɑ��݂��邩�ǂ�����Ԃ�
bool Storage::containImageData(IMAGE image)
{
	if (imageStorage[image] != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//Storage�Ɏw�肳�ꂽDescritporInfo�����łɑ��݂��邩�ǂ�����Ԃ�
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

//Storage�Ɋi�[���ꂽMeshes�̃T�C�Y��Ԃ�
uint32_t Storage::getSizeObjStorage()
{
	return meshesStorage.size();
}

//Storage�Ɋi�[���ꂽImage�̃T�C�Y��Ԃ�
uint32_t Storage::getSizeImageStorage()
{
	return imageStorage.size();
}