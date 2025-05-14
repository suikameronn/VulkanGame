#include"Storage.h"
#include"VulkanBase.h"

Storage* Storage::storage = nullptr;

Storage::Storage()
{
}

void Storage::cleanup()
{

}

//gltf���f����ǂݍ��񂾍ۂɁA���̃N���X�Ɋi�[����B
//�Ăт���gltf���f�����K�v�ɂȂ����ꍇ�́A���̃N���X����Q�Ǝ擾����
void Storage::addModel(std::string obj, GltfModel* model)
{
	gltfModelStorage[obj] = std::shared_ptr<GltfModel>(model);
}

//��Ɠ��l�A�摜��ǂݍ��񂾍ۂɂ��̃N���X�Ɋi�[����
void Storage::addImageData(std::string path,ImageData* image)
{
	imageDataStorage[path] = std::shared_ptr<ImageData>(image);
}

//VulkanBase����J�����ɃA�N�Z�X�o��悤�ɁAScene�N���X�̃J������ݒ肷��
void Storage::setCamera(std::shared_ptr<Camera> c)
{
	camera = c;
}

//�J�����ւ̎Q�Ƃ�Ԃ�
std::shared_ptr<Camera> Storage::accessCamera()
{
	return camera;
}

//���߂�ꂽ���\�[�X�����łɂ��̃N���X�Ɋi�[����Ă��邩�ǂ�����Ԃ�
//���̊֐��ł́A���̔���݂̂�S��
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

//���߂�ꂽ���\�[�X�����łɂ��̃N���X�Ɋi�[����Ă��邩�ǂ�����Ԃ�
//���̊֐��ł́A���̔���݂̂�S��
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

//���̃N���X�ɂ��łɊi�[���ꂽgltfModel��Ԃ�
std::unordered_map<std::string, std::shared_ptr<GltfModel>>& Storage::getgltfModel()
{
	return gltfModelStorage;
}

//���̃N���X�ɂ��łɊi�[���ꂽimageData��map��Ԃ�
std::unordered_map<std::string, std::shared_ptr<ImageData>>& Storage::getImageData()
{
	return imageDataStorage;
}

//���̃N���X�ɂ��łɊi�[���ꂽgltfModel��map��Ԃ�
std::shared_ptr<GltfModel> Storage::getgltfModel(std::string obj)
{
	if (!containModel(obj))
	{
		std::cerr << "Not register GltfModel" << std::endl;
	}

	return gltfModelStorage[obj];
}

//���̃N���X�ɂ��łɊi�[���ꂽ�摜��Ԃ�
std::shared_ptr<ImageData> Storage::getImageData(std::string path)
{
	if (!containImageData(path))
	{
		std::cerr << "Not register Image" << std::endl;
	}

	return imageDataStorage[path];
}