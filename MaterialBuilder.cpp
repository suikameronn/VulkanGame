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
MaterialBuilder MaterialBuilder::initProperty()
{
	property.initProperty();

	return *this;
}

//基本色のテクスチャを設定
MaterialBuilder MaterialBuilder::withBaseColorTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.baseColor = uvIndex;
	property.baseColorTexture = texture;

	return *this;
}

//メタリックと粗さのテクスチャを設定
MaterialBuilder MaterialBuilder::withMetallicRoughnessTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.metallicRoughness = uvIndex;
	property.metallicRoughnessTexture = texture;

	return *this;
}

//法線用テクスチャを設定
MaterialBuilder MaterialBuilder::withNormalTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.normal = uvIndex;
	property.normalTexture = texture;

	return *this;
}

//オクルージョン化リング用テクスチャを設定
MaterialBuilder MaterialBuilder::withOcclusionTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.occlusion = uvIndex;
	property.occlusionTexture = texture;

	return *this;
}

//発光用テクスチャを設定
MaterialBuilder MaterialBuilder::withEmissiveTexture(const int& uvIndex, std::shared_ptr<Texture> texture)
{
	property.texCoordSet.emissive= uvIndex;
	property.emissiveTexture = texture;

	return *this;
}

//基本色の設定
MaterialBuilder MaterialBuilder::withBaseColorFactor(const glm::vec4& factor)
{
	property.baseColorFactor = factor;

	return *this;
}

//発光の設定
MaterialBuilder MaterialBuilder::withEmissiveFactor(const glm::vec4& factor)
{
	property.emissiveFactor = factor;

	return *this;
}

//金属光沢の度合いを設定
MaterialBuilder MaterialBuilder::withMetallicFactor(const float& factor)
{
	property.metallicFactor = factor;

	return *this;
}

//粗さの度合いを設定
MaterialBuilder MaterialBuilder::withRoughnessFactor(const float& factor)
{
	property.roughnessFactor = factor;

	return *this;
}

//透明度の最大値を設定
MaterialBuilder MaterialBuilder::withAlphaMaskCutOff(const float& cutoff)
{
	property.alphaMaskCutoff = cutoff;

	return *this;
}

//自己放射の強さを設定
MaterialBuilder MaterialBuilder::withEmissiveStrength(const float& strength)
{
	property.emissiveStrength = strength;

	return *this;
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
	std::shared_ptr<Material> material = std::make_shared<Material>(property);

	return material;
}