#pragma once

#include"GltfModelComp.h"

#include"GltfModelFactory.h"

namespace Load
{
	void LoadModel(GltfModelComp& comp,std::shared_ptr<GltfModelFactory> factory)
	{
		//モデルをIDから取得
		std::shared_ptr<GltfModel> model = factory->GetModel(comp.modelID);

		if (model)
		{
			model->createBuffer();//モデルのバッファを作成
			model->createDescriptorSet();//モデルのディスクリプタセットを作成
		}
	}
}