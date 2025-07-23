#pragma once

#include"GltfModelComp.h"

#include"GltfModelFactory.h"

namespace Load
{
	void LoadModel(GltfModelComp& comp,std::shared_ptr<GltfModelFactory> factory)
	{
		//���f����ID����擾
		std::shared_ptr<GltfModel> model = factory->GetModel(comp.modelID);

		if (model)
		{
			model->createBuffer();//���f���̃o�b�t�@���쐬
			model->createDescriptorSet();//���f���̃f�B�X�N���v�^�Z�b�g���쐬
		}
	}
}