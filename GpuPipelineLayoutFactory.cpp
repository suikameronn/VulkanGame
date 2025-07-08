#include"GpuPipelineLayoutFactory.h"

GpuPipelineLayoutFactory::GpuPipelineLayoutFactory(VkDevice& d, std::shared_ptr<GpuPipelineLayoutBuilder> b
	, std::shared_ptr<GpuDescriptorSetLayoutFactory> layoutF)
{
	device = d;

	builder = b;

	this->layoutFactory = layoutF;
}

GpuPipelineLayoutFactory::~GpuPipelineLayoutFactory()
{
	for (auto& itr : pipelineLayoutStorage)
	{
		if (!itr.second.expired())
		{
			//本来はここで破棄されるものはないはず

			itr.second.lock().reset();
		}
	}
}

//既定のレイアウトから構造体を設定する
void GpuPipelineLayoutFactory::convertLayouts(PipelineLayoutPattern pattern
	, std::vector<std::shared_ptr<DescriptorSetLayout>>& layouts, std::vector<VkPushConstantRange>& pushConstant)
{
	if (pattern == PipelineLayoutPattern::PBR)
	{
		//PBRシェーダ

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::MVPANIM));
		builder->addLayout(layoutFactory->Create(LayoutPattern::MATERIAL));
		builder->addLayout(layoutFactory->Create(LayoutPattern::LIGHT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::LIGHT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));

		VkPushConstantRange p1{};
		p1.offset = 0;
		p1.size = sizeof(FragmentParam);
		p1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		pushConstant.push_back(p1);
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
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
	}
	else if (pattern == PipelineLayoutPattern::CALCSHADOWMAP)
	{
		//シャドウマップの計算用

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::MVPANIM));
	}
	else if (pattern == PipelineLayoutPattern::CALCCUBEMAP)
	{
		//キューブマップの計算用

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
	}
	else if (pattern == PipelineLayoutPattern::CALCIBL_DIFFUSE_SPECULAR)
	{
		//IBLのディフューズとスペキュラーの計算用

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_TEX_FLAG));
	}
	else if (pattern == PipelineLayoutPattern::CALCIBL_BRDF)
	{
		//IBLのBRDF計算用

		builder->initProperty();
		builder->addLayout(layoutFactory->Create(LayoutPattern::SINGLE_UNIFORM_VERT));
	}

	//レイアウトの構造体を取得する
	builder->Create(layouts);
}

//ビルダーでパイプラインレイアウトを作成する
std::shared_ptr<PipelineLayout> GpuPipelineLayoutFactory::createLayout(std::vector<std::shared_ptr<DescriptorSetLayout>>& layoutStruct
	, std::vector<VkPushConstantRange>& pushConstants)
{
	//構造体からVkDescriptorSetLayoutの配列を取得する
	std::vector<VkDescriptorSetLayout> layouts(layoutStruct.size());
	for (int i = 0; i < layouts.size(); i++)
	{
		layouts[i] = layoutStruct[i]->layout;
	}

	//構造体のファクトリーを設定しておく
	std::shared_ptr<PipelineLayout> pLayout
		= std::make_shared<PipelineLayout>(shared_from_this());

	//実際にパイプラインレイアウトを作成する
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
	pipelineLayoutInfo.pSetLayouts = layouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
	pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();

	//パイプラインレイアウトを作成する
	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pLayout->pLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	pLayout->layouts = layoutStruct;

	return pLayout;
}

//パイプラインレイアウトの作成
std::shared_ptr<PipelineLayout> GpuPipelineLayoutFactory::Create(std::vector<std::shared_ptr<DescriptorSetLayout>>& layouts
	, std::vector<VkPushConstantRange>& pushConstants, VkPipelineLayout& pLayout)
{
	if (layouts.size() == 0)
	{
		return std::shared_ptr<PipelineLayout>();
	}

	//すでに作成されていないか調べる
	std::weak_ptr<PipelineLayout> preCreate = pipelineLayoutStorage[{layouts, pushConstants}];
	if (!preCreate.expired())
	{
		return preCreate.lock();
	}

	//実際にパイプラインレイアウトを作成する
	std::shared_ptr<PipelineLayout> layout = createLayout(layouts, pushConstants);

	//ハッシュ値を持たせる
	layout->hashKey = pipelineLayoutHash({ layouts, pushConstants });

	//レイアウトを登録しておく
	pipelineLayoutStorage[{layouts, pushConstants}] = layout;

	return layout;
}

//パイプラインレイアウトの作成
std::shared_ptr<PipelineLayout> GpuPipelineLayoutFactory::Create(PipelineLayoutPattern pattern, VkPipelineLayout& pLayout)
{
	std::vector<std::shared_ptr<DescriptorSetLayout>> layouts;
	std::vector<VkPushConstantRange> pushConstants;

	convertLayouts(pattern, layouts, pushConstants);

	//すでに作成されていないか調べる
	std::weak_ptr<PipelineLayout> preCreate = pipelineLayoutStorage[{layouts, pushConstants}];
	if (!preCreate.expired())
	{
		return preCreate.lock();
	}

	//実際にパイプラインレイアウトを作成する
	std::shared_ptr<PipelineLayout> layout = createLayout(layouts, pushConstants);

	//ハッシュ値を持たせる
	layout->hashKey = pipelineLayoutHash({ layouts,pushConstants });

	//レイアウトを登録しておく
	pipelineLayoutStorage[{layouts, pushConstants}] = layout;

	return layout;
}

//遅延破棄リストにリソースを追加する
void GpuPipelineLayoutFactory::addDefferedDestruct(VkPipelineLayout& pLayout)
{
	destructList[frameIndex].push_back(pLayout);
}

//リソースを破棄する
void GpuPipelineLayoutFactory::resourceDestruct()
{
	//実際にリソースを破棄する
	for (auto& playout : destructList[frameIndex])
	{
		vkDestroyPipelineLayout(device, playout, nullptr);
	}

	//フレームインデックスを更新する
	frameIndex = (frameIndex == 0) ? 1 : 0;
}