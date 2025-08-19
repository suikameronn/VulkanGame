#include"MaterialBuilder.h"

void MaterialBuilder::createEmptyTexture()
{
	const uint32_t texChannel = 4;
	const uint32_t size = 2;
	const std::vector<unsigned char> pixels(size * size, 0);

	emptyTexture = textureFactory->Create(texChannel,pixels.data()
		, size, size, TexturePattern::NORMAL);
}

//マテリアルのプロパティを初期化
std::shared_ptr<MaterialBuilder> MaterialBuilder::initProperty()
{
	property.initProperty();

	return shared_from_this();
}

//基本色のテクスチャを設定
std::shared_ptr<MaterialBuilder> MaterialBuilder::withBaseColorTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.baseColor = uvIndex;
	property.baseColorTexture = texture;

	return shared_from_this();
}

//メタリックと粗さのテクスチャを設定
std::shared_ptr<MaterialBuilder> MaterialBuilder::withMetallicRoughnessTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.metallicRoughness = uvIndex;
	property.metallicRoughnessTexture = texture;

	return shared_from_this();
}

//法線用テクスチャを設定
std::shared_ptr<MaterialBuilder> MaterialBuilder::withNormalTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.normal = uvIndex;
	property.normalTexture = texture;

	return shared_from_this();
}

//オクルージョン化リング用テクスチャを設定
std::shared_ptr<MaterialBuilder> MaterialBuilder::withOcclusionTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.occlusion = uvIndex;
	property.occlusionTexture = texture;

	return shared_from_this();
}

//発光用テクスチャを設定
std::shared_ptr<MaterialBuilder> MaterialBuilder::withEmissiveTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.emissive= uvIndex;
	property.emissiveTexture = texture;

	return shared_from_this();
}

//基本色の設定
std::shared_ptr<MaterialBuilder> MaterialBuilder::withBaseColorFactor(const glm::vec4& factor)
{
	property.baseColorFactor = factor;

	return shared_from_this();
}

//発光の設定
std::shared_ptr<MaterialBuilder> MaterialBuilder::withEmissiveFactor(const glm::vec4& factor)
{
	property.emissiveFactor = factor;

	return shared_from_this();
}

//金属光沢の度合いを設定
std::shared_ptr<MaterialBuilder> MaterialBuilder::withMetallicFactor(const float& factor)
{
	property.metallicFactor = factor;

	return shared_from_this();
}

//粗さの度合いを設定
std::shared_ptr<MaterialBuilder> MaterialBuilder::withRoughnessFactor(const float& factor)
{
	property.roughnessFactor = factor;

	return shared_from_this();
}

//透明度の最大値を設定
std::shared_ptr<MaterialBuilder> MaterialBuilder::withAlphaMaskCutOff(const float& cutoff)
{
	property.alphaMaskCutoff = cutoff;

	return shared_from_this();
}

//自己放射の強さを設定
std::shared_ptr<MaterialBuilder> MaterialBuilder::withEmissiveStrength(const float& strength)
{
	property.emissiveStrength = strength;

	return shared_from_this();
}

//マテリアルのプロパティを作成
MaterialProperty MaterialBuilder::Build()
{
	property.Build(emptyTexture);

	MaterialProperty prop = property;

	initProperty();

	return prop;
}

//マテリアルを作成
std::shared_ptr<Material> MaterialBuilder::Create(const MaterialProperty& property)
{
	std::shared_ptr<Material> material = std::make_shared<Material>(property, bufferFactory, layoutFactory, descriptorSetFactory);

	return material;
}