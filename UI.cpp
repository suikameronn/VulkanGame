#include"UI.h"

#include"VulkanBase.h"
#include"GameManager.h"

UI::UI(std::shared_ptr<ImageData> image)
{
	uiNum = UINum::IMAGE;

	exist = true;

	uiImage = image;

	GameManager* manager = GameManager::GetInstance();

	projMatrix = manager->getUIProjection();

	isUpdateTransformMatrix = true;

	position = glm::vec2(0.0f ,0.0f);

	rotate.z = 0.0f;

	scale = 1.0f;

	isVisible = true;

	vertices.resize(UIVertexCount);

	//UIの画像を張り付けるためのポリゴンの頂点の座標を設定
	vertices[0].uv = glm::vec2(0.0f);
	vertices[1].uv = glm::vec2(1.0f, 0.0f);
	vertices[2].uv = glm::vec2(0.0f, 1.0f);
	vertices[3].uv = glm::vec2(1.0f);

	indices.resize(UIIndexCount);
	indices = { 0, 1, 2, 
				2, 3, 1 };

	aspect = glm::vec2(1.0f);

	uiWidth = 1.0f;
	uiHeight = 1.0f;

	//UIを表示するための行列を記録するためのバッファを作成
	VulkanBase::GetInstance()->uiCreateUniformBuffer(mappedBuffer);
}

//2D上の座標を設定
void UI::setPosition(glm::vec2 pos)
{
	position = pos;

	isUpdateTransformMatrix = true;
}

//2D上の回転を設定
void UI::setRotate(Rotate2D rot)
{
	rotate = rot;
	isUpdateTransformMatrix = true;
}

//2D上のスケールを設定
void UI::setScale(float scale)
{
	this->scale = scale;
	isUpdateTransformMatrix = true;

	uiWidth = aspect[0] * scale;
	uiHeight = aspect[1] * scale;
}

//UIの表示・非表示を設定
void UI::setVisible(bool visible)
{
	isVisible = visible;
}

//頂点配列を返す
Vertex2D* UI::getVertices()
{
	return vertices.data();
}

//インデックス配列を返す
uint32_t* UI::getIndices()
{
	return indices.data();
}

//gpu上の頂点バッファを返す
BufferObject* UI::getPointBuffer()
{
	return pointBuffers.data();
}

//uiの画像のテクスチャを返す
TextureData* UI::getUITexture()
{
	return uiImage->getTexture();
}

//uiの画像を返す
std::shared_ptr<ImageData> UI::getImageData()
{
	return uiImage;
}

//uiの画像をgpu上に展開し、テクスチャを作成
void UI::createTexture(std::shared_ptr<ImageData> image) 
{
	uiImage = image;

	VulkanBase* vulkan = VulkanBase::GetInstance();

	//ui画像の縦横比を計算
	float commonDivide = static_cast<float>(std::gcd(uiImage->getWidth(), uiImage->getHeight()));

	aspect[0] = uiImage->getWidth() / commonDivide;
	aspect[1] = uiImage->getHeight() / commonDivide;

	uiWidth = aspect[0] * scale;
	uiHeight = aspect[1] * scale;

	//uiの縦横比からポリゴンの座標を設定
	vertices[0].pos = glm::vec2(0.0f,0.0f);
	vertices[1].pos = glm::vec2(aspect[0], 0.0f);
	vertices[2].pos = glm::vec2(0.0f, aspect[1]);
	vertices[3].pos = glm::vec2(aspect[0], aspect[1]);
}

//uiを表示する座標変換行列を更新
void UI::updateTransformMatrix()
{
	transformMatrix = glm::mat4(1.0f);

	//2D用の行列を作成
	transformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f))
		* rotate.getRotateMatrix()
		* glm::translate(glm::mat4(1.0f), glm::vec3(-uiWidth / 2.0f, -uiHeight / 2.0f, 0.0f))
		* glm::scale(glm::mat4(1.0f), glm::vec3(scale));

	isUpdateTransformMatrix = false;
}

//uiの座標変換行列を更新
void UI::initFrameSettings()
{
	//UIの画像をgpu上に展開
	createTexture(uiImage);

	updateTransformMatrix();
}

//gpu上のバッファを破棄
void UI::cleanupVulkan()
{
	VulkanBase* vulkan = VulkanBase::GetInstance();

	for (BufferObject& pointBuffer : pointBuffers)
	{
		vulkan->addDefferedDestructBuffer(pointBuffer);
	}
	vulkan->addDefferedDestructBuffer(mappedBuffer);
}

void UI::updateUniformBuffer()
{
	MatricesUBO2D ubo{};
	ubo.transformMatrix = transformMatrix;
	ubo.projection = projMatrix;

	memcpy(mappedBuffer.uniformBufferMapped, &ubo, sizeof(ubo));
}

void UI::frameEnd()
{
	updateUniformBuffer();
}