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
	
	//ロード画面用のUI
	std::shared_ptr<UI> loadUI;

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

	//gltfモデルを読み込んだ際に、このクラスに格納する。
	//再びそのgltfモデルが必要になった場合は、このクラスから参照取得する
	void addModel(std::string obj, GltfModel* geo);
	//上と同様、画像を読み込んだ際にこのクラスに格納する
	void addImageData(std::string, ImageData* image);

	//VulkanBaseからカメラにアクセス出るように、Sceneクラスのカメラを設定する
	void setCamera(std::shared_ptr<Camera> c);

	//このクラスにすでに格納されたgltfModelを返す
	std::shared_ptr<GltfModel> getgltfModel(std::string obj);
	//このクラスにすでに格納された画像を返す
	std::shared_ptr<ImageData> getImageData(std::string path);
	//このクラスにすでに格納されたgltfModelのmapを返す
	std::unordered_map<std::string, std::shared_ptr<GltfModel>>& getgltfModel();
	//このクラスにすでに格納されたimageDataのmapを返す
	std::unordered_map<std::string, std::shared_ptr<ImageData>>& getImageData();

	//カメラへの参照を返す
	std::shared_ptr<Camera> accessCamera();

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