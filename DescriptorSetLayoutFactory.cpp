#include"DescriptorSetLayoutFactory.h"

DescriptorSetLayoutFactory::DescriptorSetLayoutFactory(VkDevice& d, std::shared_ptr<DescriptorSetLayoutBuilder> b)
{

	device = d;

	builder = b;

	frameIndex = 1;
}


DescriptorSetLayoutFactory::~DescriptorSetLayoutFactory()
{
	for (auto& itr : layoutStorage)
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
	std::cout << "DescriptorSetLayoutFactory :: デストラクタ" << std::endl;
#endif
}

//レイアウトの作成
std::shared_ptr<DescriptorSetLayout> DescriptorSetLayoutFactory::createLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	//構造体のファクトリーを設定しておく
	std::shared_ptr<DescriptorSetLayout> layout
		= std::make_shared<DescriptorSetLayout>(shared_from_this());

	//実際にレイアウトを作成する
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout->layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	return layout;
}

//既定のレイアウトからビンディングを返す
void DescriptorSetLayoutFactory::convertBinding(const LayoutPattern& pattern,std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	if (pattern == LayoutPattern::MODELANIMMAT)
	{
		//頂点シェーダとコンピュートシェーダ
		//行列用のユニフォームバッファ
		//このセットを二つ
		//それぞれmvp行列とアニメーション行列を入れる

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT);
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT);
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT);
	}
	else if (pattern == LayoutPattern::CAMERA)
	{
		//頂点シェーダとフラグメントシェーダ
		//カメラの座標、ビュー行列、プロジェクション行列を入れる

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	else if (pattern == LayoutPattern::MATERIAL)
	{
		//すべてのフラグメントシェーダ
		//テクスチャの影響度の構造体と5枚のテクスチャ

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);

		for (int i = 0; i < MAX_TEXTURE_COUNT; i++)
		{
			builder->setProperty(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		}
	}
	else if (pattern == LayoutPattern::LIGHT)
	{
		//フラグメントシェーダ
		//ライトの色や位置を記録したユニフォームバッファ.

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	else if(pattern == LayoutPattern::SHADOWMAP)
	{
		//フラグメントシェーダ
		//ライトの色や位置を記録したユニフォームバッファとシャドウマップの配列

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

		builder->setProperty(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		builder->setProperty(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	else if (pattern == LayoutPattern::CUBEMAP)
	{
		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	else if (pattern == LayoutPattern::IBL)
	{
		//フラグメントシェーダ
		//IBLのディヒューズ、スペキュラー、BRDFを含む三つのテクスチャ
		
		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		builder->setProperty(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		builder->setProperty(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	else if (pattern == LayoutPattern::RAYCAST)
	{
		//すべてコンピュートシェーダ
		//頂点バッファとインデックスバッファをストレージバッファとして結び付ける

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT);
		builder->setProperty(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT);
	}
	else if (pattern == LayoutPattern::UI)
	{
		//頂点シェーダとフラグメントシェーダ
		//UIのテクスチャを張り付けるポリゴンの座標変換行列と画像を結び付ける

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
		builder->setProperty(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	else if (pattern == LayoutPattern::SINGLE_TEX_FLAG)
	{
		//フラグメントシェーダ
		//単一のテクスチャを結び付ける

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

	}
	else if (pattern == LayoutPattern::SINGLE_UNIFORM_VERT)
	{
		//頂点シェーダ
		//単一のユニフォームバッファを結び付ける

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
	}
	else if (pattern == LayoutPattern::SINGLE_UNIFORM_FRAG)
	{
		//フラグメントシェーダ
		//単一のユニフォームバッファを結び付ける

		builder->initProperty();
		builder->setProperty(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	//バインディングを作成する
	bindings = builder->Build();
}

//レイアウトの作成
std::shared_ptr<DescriptorSetLayout> DescriptorSetLayoutFactory::Create(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	if (bindings.size() == 0)
	{
		return std::shared_ptr<DescriptorSetLayout>();
	}

	//同一のレイアウトが既に作成されていないかを調べる
	std::weak_ptr<DescriptorSetLayout> preCreateLayout = layoutStorage[bindings];
	if (!preCreateLayout.expired())
	{
		return preCreateLayout.lock();
	}

	//実際にレイアウトを作成する
	std::shared_ptr<DescriptorSetLayout> layout = createLayout(bindings);

	//ハッシュ値を持たせる
	layout->hashKey = layoutHash.genHash(bindings);

	//レイアウトを登録しておく
	layoutStorage[bindings] = layout;

	return layout;
}

//既定のレイアウトの作成
std::shared_ptr<DescriptorSetLayout> DescriptorSetLayoutFactory::Create(const LayoutPattern& pattern)
{
	//enum classからバインディングを作る
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	convertBinding(pattern, bindings);

	//同一のレイアウトが既に作成されていないかを調べる
	std::weak_ptr<DescriptorSetLayout> preCreateLayout = layoutStorage[bindings];
	if (!preCreateLayout.expired())
	{
		return preCreateLayout.lock();
	}

	//実際にレイアウトを作成する
	std::shared_ptr<DescriptorSetLayout> layout = createLayout(bindings);

	//ハッシュ値を持たせる
	layout->hashKey = layoutHash.genHash(bindings);

	//レイアウトを登録しておく
	layoutStorage[bindings] = layout;

	return layout;
}

//遅延破棄リストにリソースを追加する
void DescriptorSetLayoutFactory::addDefferedDestruct(VkDescriptorSetLayout& layout)
{
	destructList[frameIndex].push_back(layout);
}

//リソースを破棄する
void DescriptorSetLayoutFactory::resourceDestruct()
{
	//フレームインデックスを更新する
	frameIndex = (frameIndex == 0) ? 1 : 0;

	//実際にリソースを破棄する
	for (auto& layout : destructList[frameIndex])
	{
		vkDestroyDescriptorSetLayout(device, layout, nullptr);
	}

	destructList[frameIndex].clear();
}