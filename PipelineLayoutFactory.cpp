#include"PipelineLayoutFactory.h"

PipelineLayoutFactory::PipelineLayoutFactory(VkDevice& d, std::shared_ptr<PipelineLayoutBuilder> b
	, std::shared_ptr<DescriptorSetLayoutFactory> layoutF)
{
	frameIndex = 1;

	device = d;

	builder = b;

	this->layoutFactory = layoutF;
}

PipelineLayoutFactory::~PipelineLayoutFactory()
{
	for (auto& itr : pipelineLayoutStorage)
	{
		if (!itr.second.expired())
		{
			//本来はここで破棄されるものはないはず

			itr.second.lock().reset();
		}
	}

	for (int i = 0; i < 2; i++)
	{
		resourceDestruct();
		resourceDestruct();
	}

#ifdef _DEBUG
	std::cout << "PipelineLayoutFactory :: デストラクタ" << std::endl;
#endif
}

//既定のレイアウトから構造体を設定する
PipelineLayoutProperty PipelineLayoutFactory::convertLayouts(const PipelineLayoutPattern& pattern)
{
	if (pattern == PipelineLayoutPattern::PBR)
	{
		//PBRシェーダ

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::MODELANIMMAT));//モデル行列
		builder->addLayout(layoutFactory->Create(LayoutPattern::CAMERA));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SHADOWMAP));
		builder->addLayout(layoutFactory->Create(LayoutPattern::MATERIAL));
		builder->addLayout(layoutFactory->Create(LayoutPattern::IBL));

		builder->addPushConstant(sizeof(FragmentParam), VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	else if (pattern == PipelineLayoutPattern::UI)
	{
		//UIレンダリング用

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
	}
	else if (pattern == PipelineLayoutPattern::CUBEMAP)
	{
		//キューブマップレンダリング

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::CAMERA));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
	}
	else if (pattern == PipelineLayoutPattern::CALC_SHADOWMAP)
	{
		//シャドウマップの計算用

		builder->initProperty();

		const std::shared_ptr<DescriptorSetLayout> layout = layoutFactory->Create
		(
			layoutFactory->getBuilder()
			->initProperty()
			->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT)
			->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT)
			->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT)
			->Build()
		);

		builder->addLayout(layout);
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));

		builder->addPushConstant(sizeof(uint32_t), VK_SHADER_STAGE_VERTEX_BIT);
	}
	else if (pattern == PipelineLayoutPattern::CALC_CUBEMAP)
	{
		//キューブマップの計算用

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
	}
	else if (pattern == PipelineLayoutPattern::CALC_IBL_DIFFUSE)
	{
		//IBLのディフューズの計算用

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
	}
	else if (pattern == PipelineLayoutPattern::CALC_IBL_SPECULAR)
	{
		//IBLのスペキュラーの計算用

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
		
		builder->addPushConstant(sizeof(SpecularPushConstant), VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	else if (pattern == PipelineLayoutPattern::CALC_IBL_BRDF)
	{
		//IBLのBRDF計算用

		builder->initProperty();
	}
	else if(pattern == PipelineLayoutPattern::RAYCAST)
	{
		std::shared_ptr<DescriptorSetLayout> raycastLayout = layoutFactory->Create(layoutFactory->getBuilder()
			->initProperty()
			->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			->setProperty(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			->Build());

		//レイキャスト用
		builder->initProperty();
		builder->addLayout(raycastLayout)
			->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT))
			->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT))
			->addLayout(layoutFactory->Create(LayoutPattern::RAYCAST));

		builder->addPushConstant(sizeof(RaycastPushConstant), VK_SHADER_STAGE_COMPUTE_BIT);
	}
	else if (pattern == PipelineLayoutPattern::COLIDER)
	{
		//コライダー描画用

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));//モデル行列
		builder->addLayout(layoutFactory->Create(LayoutPattern::CAMERA));
	}

	//レイアウトの構造体を取得する
	return builder->Build();
}

//ビルダーでパイプラインレイアウトを作成する
std::shared_ptr<PipelineLayout> PipelineLayoutFactory::createLayout(const PipelineLayoutProperty& property)
{
	//構造体からVkDescriptorSetLayoutの配列を取得する
	std::vector<VkDescriptorSetLayout> layouts(property.layoutArray.size());
	for (int i = 0; i < layouts.size(); i++)
	{
		layouts[i] = property.layoutArray[i]->layout;
	}

	//構造体のファクトリーを設定しておく
	std::shared_ptr<PipelineLayout> pLayout
		= std::make_shared<PipelineLayout>(shared_from_this());

	//実際にパイプラインレイアウトを作成する
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
	pipelineLayoutInfo.pSetLayouts = layouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(property.pushconstantArray.size());
	pipelineLayoutInfo.pPushConstantRanges = property.pushconstantArray.data();

	//パイプラインレイアウトを作成する
	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pLayout->pLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	pLayout->layouts = property.layoutArray;
	pLayout->pushconstants = property.pushconstantArray;

	return pLayout;
}

//パイプラインレイアウトの作成
std::shared_ptr<PipelineLayout> PipelineLayoutFactory::Create(const PipelineLayoutProperty& property)
{
	if (property.layoutArray.size() == 0)
	{
		return std::shared_ptr<PipelineLayout>();
	}

	//すでに作成されていないか調べる
	std::weak_ptr<PipelineLayout> preCreate = pipelineLayoutStorage[property];
	if (!preCreate.expired())
	{
		return preCreate.lock();
	}

	//実際にパイプラインレイアウトを作成する
	std::shared_ptr<PipelineLayout> layout = createLayout(property);

	//ハッシュ値を持たせる
	layout->hashKey = pipelineLayoutHash(property);

	//レイアウトを登録しておく
	pipelineLayoutStorage[property] = layout;

	return layout;
}

//パイプラインレイアウトの作成
std::shared_ptr<PipelineLayout> PipelineLayoutFactory::Create(const PipelineLayoutPattern& pattern)
{
	std::vector<std::shared_ptr<DescriptorSetLayout>> layouts;
	std::vector<VkPushConstantRange> pushConstants;

	const PipelineLayoutProperty& property = convertLayouts(pattern);

	//すでに作成されていないか調べる
	std::weak_ptr<PipelineLayout> preCreate = pipelineLayoutStorage[property];
	if (!preCreate.expired())
	{
		return preCreate.lock();
	}

	//実際にパイプラインレイアウトを作成する
	std::shared_ptr<PipelineLayout> layout = createLayout(property);

	//ハッシュ値を持たせる
	layout->hashKey = pipelineLayoutHash(property);

	//レイアウトを登録しておく
	pipelineLayoutStorage[property] = layout;

	return layout;
}

//遅延破棄リストにリソースを追加する
void PipelineLayoutFactory::addDefferedDestruct(VkPipelineLayout& pLayout)
{
	destructList[frameIndex].push_back(pLayout);
}

//リソースを破棄する
void PipelineLayoutFactory::resourceDestruct()
{
	//フレームインデックスを更新する
	frameIndex = (frameIndex == 0) ? 1 : 0;

	//実際にリソースを破棄する
	for (auto& playout : destructList[frameIndex])
	{
		vkDestroyPipelineLayout(device, playout, nullptr);
	}

	destructList[frameIndex].clear();
}