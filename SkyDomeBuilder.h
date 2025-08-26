#pragma once

#include<iostream>
#include"TextureFactory.h"
#include"DescriptorSetFactory.h"

#include"PipelineLayoutFactory.h"
#include"PipelineFactory.h"

#include"GltfModelFactory.h"

#include"Render.h"

#include"CameraComp.h"

#include"StructList.h"

#define CUBEMAP_LAYER 6

enum class CUBEMAP_VIEW
{
	FRONT = 0,
	BACK,
	RIGHT,
	LEFT,
	TOP,
	BOTTOM
};

struct BackGroundColor
{
	//レイヤーが6つのレンダリングで利用するテクスチャ
	std::shared_ptr<Texture> multiLayerTex;

	BackGroundColor()
	{
		init();
	}

	void init()
	{
		multiLayerTex.reset();
	}
};

struct DiffuseMap
{
	//レイヤーが6つのレンダリングで利用するテクスチャ
	std::shared_ptr<Texture> multiLayerTex;

	DiffuseMap()
	{
		init();
	}

	void init()
	{
		multiLayerTex.reset();
	}
};

struct ReflectionMap
{
	//レイヤーが6つのレンダリングで利用するテクスチャ
	std::shared_ptr<Texture> multiLayerTex;

	ReflectionMap()
	{
		init();
	}

	void init()
	{
		multiLayerTex.reset();
	}
};

struct BRDFMap
{
	//これのレイヤーは一つ
	std::shared_ptr<Texture> texture;

	BRDFMap()
	{
		init();
	}

	void init()
	{
		texture.reset();
	}
};

struct SkyDome
{
	BackGroundColor backGround;
	DiffuseMap diffuse;
	ReflectionMap reflection;
	BRDFMap brdf;

	std::shared_ptr<DescriptorSet> cubemapDescriptorSet;
	std::shared_ptr<DescriptorSet> iblDescriptorSet;
};

struct SkyDomeProperty
{
	//IBLの元となる画像ファイルのパス
	std::string srcImagePath;

	SkyDomeProperty()
	{
		initProperty();
	}

	void initProperty()
	{
		srcImagePath = "";
	}
};

class SkyDomeBuilder : public std::enable_shared_from_this<SkyDomeBuilder>
{
private:

	std::shared_ptr<VulkanCore> vulkanCore;

	std::shared_ptr<GpuBufferFactory> bufferFactory;

	std::shared_ptr<TextureFactory> textureFactory;

	std::shared_ptr<FrameBufferFactory> frameBufferFactory;

	std::shared_ptr<DescriptorSetLayoutFactory> layoutFactory;
	std::shared_ptr<DescriptorSetFactory> descriptorSetFactory;

	std::shared_ptr<PipelineLayoutFactory> pipelineLayoutFactory;
	std::shared_ptr<PipelineFactory> pipelineFactory;

	std::shared_ptr<RenderPassFactory> renderPassFactory;

	uint32_t cubemapModelID;
	CameraUniform camera;

	std::shared_ptr<GltfModelFactory> modelFactory;

	std::shared_ptr<CommandBufferFactory> commandFactory;

	std::shared_ptr<Render> render;

	SkyDomeProperty property;

	void createBackGround(const std::shared_ptr<SkyDome> skydome, const std::shared_ptr<Texture> srcTexture
		, const std::array<std::shared_ptr<DescriptorSet>, CUBEMAP_LAYER>& uniform, const std::shared_ptr<DescriptorSet> srcTex);
	
	void createDiffuse(const std::shared_ptr<SkyDome> skydome, const std::shared_ptr<Texture> srcTexture
		, const std::array<std::shared_ptr<DescriptorSet>, CUBEMAP_LAYER>& uniform, const std::shared_ptr<DescriptorSet> srcTex);
	
	void createReflection(const std::shared_ptr<SkyDome> skydome, const std::shared_ptr<Texture> srcTexture
		, const std::array<std::shared_ptr<DescriptorSet>, CUBEMAP_LAYER>& uniform, const std::shared_ptr<DescriptorSet> srcTex);
	
	void createBRDF(const std::shared_ptr<SkyDome> skydome, const std::shared_ptr<Texture> srcTexture
		, const std::array<std::shared_ptr<DescriptorSet>, CUBEMAP_LAYER>& uniform);

	void switchView(std::shared_ptr<DescriptorSet> uniform, const int index);

public:

	SkyDomeBuilder(std::shared_ptr<VulkanCore> core, std::shared_ptr<GpuBufferFactory> buffer, std::shared_ptr<TextureFactory> texture
		, std::shared_ptr<FrameBufferFactory> frame, std::shared_ptr<PipelineLayoutFactory> pLayout
		, std::shared_ptr<PipelineFactory> pipeline, std::shared_ptr<RenderPassFactory> renderPass
		, std::shared_ptr<DescriptorSetLayoutFactory> layout, std::shared_ptr<DescriptorSetFactory> descriptorSet
		, std::shared_ptr<Render> render, std::shared_ptr<GltfModelFactory> modelFactory
		, std::shared_ptr<CommandBufferFactory> command);

	~SkyDomeBuilder()
	{
#ifdef _DEBUG
		std::cout << "SkyDomeBuilder :: デストラクタ" << std::endl;
#endif
	}

	std::shared_ptr<SkyDomeBuilder> initProperty()
	{
		property.initProperty();

		return shared_from_this();
	}

	//マップの元となる画像ファイルを設定する
	std::shared_ptr<SkyDomeBuilder> withImagePath(const std::string path);

	//プロパティを返す
	SkyDomeProperty Build()
	{
		return property;
	}

	std::shared_ptr<SkyDome> Create(const SkyDomeProperty& prop);

	uint32_t getCubemapModelID()
	{
		return cubemapModelID;
	}
};