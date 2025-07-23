#pragma once

#include"Material.h"

#include"TextureFactory.h"

class MaterialBuilder
{
private:

	//ファクトリークラス
	std::shared_ptr<GpuBufferFactory> bufferFactory;
	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;
	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;
	std::shared_ptr<TextureFactory> textureFactory;

	std::shared_ptr<Texture> emptyTexture;

	MaterialProperty property;

	void createEmptyTexture();

public:

	MaterialBuilder(std::shared_ptr<GpuBufferFactory> buffer,
		std::shared_ptr<DescriptorSetLayoutFactory> layout,
		std::shared_ptr<DescriptorSetFactory> desc
		, std::shared_ptr<TextureFactory> texture)
	{
		bufferFactory = buffer;
		layoutFactory = layout;
		descriptorSetFactory = desc;
		textureFactory = texture;

		property.initProperty();

		//ダミーテクスチャを作る
		createEmptyTexture();
	}

	//マテリアルのプロパティを初期化
	MaterialBuilder initProperty();

	//基本色のテクスチャを設定
	MaterialBuilder withBaseColorTexture(const int& uvIndex,std::shared_ptr<Texture> texture);

	//メタリックと粗さのテクスチャを設定
	MaterialBuilder withMetallicRoughnessTexture(const int& uvIndex, std::shared_ptr<Texture> texture);

	//法線用テクスチャを設定
	MaterialBuilder withNormalTexture(const int& uvIndex, std::shared_ptr<Texture> texture);

	//オクルージョン化リング用テクスチャを設定
	MaterialBuilder withOcclusionTexture(const int& uvIndex, std::shared_ptr<Texture> texture);

	//発光用テクスチャを設定
	MaterialBuilder withEmissiveTexture(const int& uvIndex, std::shared_ptr<Texture> texture);

	//基本色の設定
	MaterialBuilder withBaseColorFactor(const glm::vec4& factor);

	//発光の設定
	MaterialBuilder withEmissiveFactor(const glm::vec4& factor);

	//金属光沢の度合いを設定
	MaterialBuilder withMetallicFactor(const float& factor);

	//粗さの度合いを設定
	MaterialBuilder withRoughnessFactor(const float& factor);

	//透明度の最大値を設定
	MaterialBuilder withAlphaMaskCutOff(const float& cutoff);

	//自己放射の強さを設定
	MaterialBuilder withEmissiveStrength(const float& strength);

	//マテリアルのプロパティを作成
	MaterialProperty Build();

	//マテリアルを作成
	std::shared_ptr<Material> Create(const MaterialProperty& property);
};