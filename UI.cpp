#include"UI.h"

#include"VulkanBase.h"
#include"GameManager.h"

UI::UI(std::shared_ptr<ImageData> image)
{
	GameManager* manager = GameManager::GetInstance();
	widthOffset = manager->window_width;
	heightOffset = manager->window_height;

	projMatrix = manager->uiProjection;

	isUpdateTransformMatrix = true;

	position = glm::vec2(0.0f ,0.0f);

	rotate.z = 0.0f;

	scale = 1.0f;

	isVisible = true;

	vertices = { glm::vec2(0.0f) };

	//長方形の頂点のuvを設定する
	vertices[0].uv = glm::vec2(0.0f);
	vertices[1].uv = glm::vec2(1.0f, 0.0f);
	vertices[2].uv = glm::vec2(0.0f, 1.0f);
	vertices[3].uv = glm::vec2(1.0f);

	indices = { 0, 1, 2, // 最初の三角形
				2, 3, 1 };  // 2番目の三角形

	aspect = glm::vec2(1.0f);

	uiWidth = 1.0f;
	uiHeight = 1.0f;

	uiTexture = new TextureData();

	//座標変換行列用のバッファの作成
	VulkanBase::GetInstance()->uiCreateUniformBuffer(mappedBuffer);

	//テクスチャの設定
	createTexture(image);
}

//座標の設定
void UI::setPosition(glm::vec2 pos)
{
	position = pos;

	isUpdateTransformMatrix = true;
}

//回転の設定
void UI::setRotate(Rotate2D rot)
{
	rotate = rot;
	isUpdateTransformMatrix = true;
}

//伸縮の設定
void UI::setScale(float scale)
{
	this->scale = scale;
	isUpdateTransformMatrix = true;

	uiWidth = aspect[0] * scale;
	uiHeight = aspect[1] * scale;
}

//表示非表示の設定 引数によって設定
void UI::setVisible(bool visible)
{
	isVisible = visible;
}

//頂点配列の取得
Vertex2D* UI::getVertices()
{
	return vertices.data();
}

//インデックス配列の取得
uint32_t* UI::getIndices()
{
	return indices.data();
}

//gpu上の頂点に関するバッファを取得
BufferObject& UI::getPointBuffer()
{
	return pointBuffer;
}

//uiとして見られるテクスチャの取得
TextureData* UI::getUITexture()
{
	return uiTexture;
}

//ui画像を返す
std::shared_ptr<ImageData> UI::getImageData()
{
	return uiImage;
}

//テクスチャ画像の設定とテクスチャの作成
void UI::createTexture(std::shared_ptr<ImageData> image) 
{
	uiImage = image;

	VulkanBase* vulkan = VulkanBase::GetInstance();
	VkDevice device = vulkan->GetDevice();

	if (uiTexture)
	{
		uiTexture->destroy(device);
		uiTexture = new TextureData();
	}

	//gpu上にテクスチャのデータを作成
	vulkan->createUITexture(uiTexture, uiImage);

	//VkDescriptorSetの用意が出来ているなら、結び付けを更新する
	if (descriptorSet)
	{
		vulkan->changeUITexture(uiTexture,mappedBuffer,descriptorSet);
	}

	//画像のアスペクト比を計算する
	float commonDivide = static_cast<float>(std::gcd(uiImage->getWidth(), uiImage->getHeight()));

	aspect[0] = uiImage->getWidth() / commonDivide;
	aspect[1] = uiImage->getHeight() / commonDivide;

	uiWidth = aspect[0] * scale;
	uiHeight = aspect[1] * scale;

	//頂点をアスペクト比に合わせて更新する
	vertices[0].pos = glm::vec2(0.0f,0.0f);
	vertices[1].pos = glm::vec2(aspect[0], 0.0f);
	vertices[2].pos = glm::vec2(0.0f, aspect[1]);
	vertices[3].pos = glm::vec2(aspect[0], aspect[1]);
}

//座標変換行列の更新
void UI::updateTransformMatrix()
{
	transformMatrix = glm::mat4(1.0f);

	transformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f))
		* rotate.getRotateMatrix() * glm::scale(glm::mat4(1.0f), glm::vec3(scale));

	isUpdateTransformMatrix = false;
}

void UI::cleanupVulkan()
{
	VkDevice device = VulkanBase::GetInstance()->GetDevice();

	vkDestroyBuffer(device, pointBuffer.vertBuffer, nullptr);
	vkFreeMemory(device, pointBuffer.vertHandler, nullptr);

	vkDestroyBuffer(device, pointBuffer.indeBuffer, nullptr);
	vkFreeMemory(device, pointBuffer.indeHandler, nullptr);

	//uniform bufferの解放
	vkDestroyBuffer(device, mappedBuffer.uniformBuffer, nullptr);
	vkFreeMemory(device, mappedBuffer.uniformBufferMemory, nullptr);
	mappedBuffer.uniformBufferMapped = nullptr;

	uiTexture->destroy(device);
}