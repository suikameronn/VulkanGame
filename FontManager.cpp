#include"VulkanBase.h"

#include"FontManager.h"

FontManager* FontManager::instance = nullptr;

FontManager::FontManager()
{
    texturePack = nullptr;

    // 1. library��������
    auto error = FT_Init_FreeType(&library);
    if (error)
    {
        throw std::runtime_error("font library initialize failed");
    }

    // 2. face���쐬
    error = FT_New_Face(library, fontPath.c_str(), 0, &face);
    if (error)
    {
        throw std::runtime_error("font face create failed");
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    slot = face->glyph;

    createAtlasTexture();
}

FontManager::~FontManager()
{
    VkDevice device = VulkanBase::GetInstance()->getDevice();

    atlasTexture->cleanUpVulkan(device);
}

//���{��̃A�g���X�e�N�X�`�������
void FontManager::createAtlasTexture()
{
    //�����̃r�b�g�}�b�v���擾����
    loadJapaneseFile();

    //�������������Ă��ׂĂ̕����̃r�b�g�}�b�v��
    //�܂ރe�N�X�`�������
    
    //�A�g���X�e�N�X�`���̏c�Ɖ��̃T�C�Y
    int atlasWidth, atlasHeight;
    //�쐬�����r�b�g�}�b�v�̐�
    int characterNum = static_cast<int>(fontMap.size());

    //�r�b�g�}�b�v�̑��ʐς��v�Z����
    int totalArea = 0;
    for (auto itr = fontMap.begin(); itr != fontMap.end(); itr++)
    {
        totalArea += itr->second.size.x * itr->second.size.y;
    }

    //�����悻�̃e�N�X�`���T�C�Y�̈�ӂ̑傫��
    int estimatedSize = static_cast<int>(std::sqrt(totalArea));

    //�e�N�X�`���̃T�C�Y���v�Z
    //�Ȃ��e�N�X�`���̃T�C�Y��2�ׂ̂���ɂ���
    int texSize = 1;
    while (texSize < estimatedSize)
    {
        texSize *= 2;
    }

    //�A�g���X�e�N�X�`���̃T�C�Y������
    atlasWidth = texSize;
    atlasHeight = texSize;

    //�e�N�X�`���p�b�L���O���J�n����
    texturePack = std::make_unique<MaxRectPacking>(atlasWidth, atlasHeight);

    //�e�N�X�`���p�b�N�ɑ}�����Ă���
    //�p�b�L���O�Ɏ��s�����ꍇ�́A�A�g���X�e�N�X�`���̃T�C�Y��傫������
    bool packingSuccess = true;
    for (auto itr = fontMap.begin(); itr != fontMap.end(); itr++)
    {
        itr->second.rect = texturePack->insert(packingSuccess,itr->second.size.x, itr->second.size.y);
    }

    //�e�N�X�`���p�b�L���O���I������
    texturePack.reset();

    //���������uv�Ȃǂ�ݒ肵�Ă���
    for (auto itr = fontMap.begin(); itr != fontMap.end(); itr++)
    {
        itr->second.uvSet(atlasWidth, atlasHeight);
    }

    //���ׂĂ̕����̃r�b�g�}�b�v���A�g���X�e�N�X�`���ɂ܂Ƃ߂�
    packBitmapsToAtlas(atlasWidth, atlasHeight);
}

//���{��̈ꗗ�𕶎���Ƃ��ēǂݍ���
void FontManager::loadJapaneseFile()
{
    std::ifstream ifs(characterPath);

    if (!ifs)
    {
        throw std::runtime_error("failed load character file");
    }

    //���ׂĂ̕����̃r�b�g�}�b�v���쐬����
    std::string line;
    while (std::getline(ifs, line))
    {
        const char* begin = line.c_str();
        const char* end = begin + strlen(line.c_str());

        while (begin < end)
        {
            uint32_t code = utf8::next(begin, end);

            createBitmap(code);
        }
    }

    //�t�@�C�������
    ifs.close();
}

//�r�b�g�}�b�v���쐬
void FontManager::createBitmap(const uint32_t c)
{
    if (FT_Load_Char(face, c, FT_LOAD_RENDER))
    {
        return;
    }

    FT_GlyphSlot& glyph = face->glyph;

    //�t�H���g�̃I�t�Z�b�g�Ȃǂ�ݒ�
    CharFont font;
    font.size = glm::vec2(glyph->bitmap.width + 2 * PADDING_PIXEL, glyph->bitmap.rows + 2 * PADDING_PIXEL);
    font.bearing = glm::vec2(glyph->bitmap_left, glyph->bitmap_top);
    font.advance = static_cast<uint32_t>(glyph->advance.x);
    fontMap[c] = font;

    bitmaps[c] = std::make_shared<ImageData>(glyph->bitmap);
}

//���ׂĂ̕����̃r�b�g�}�b�v���A�g���X�e�N�X�`���ɂ܂Ƃ߂�
void FontManager::packBitmapsToAtlas(int atlasWidth, int atlasHeight)
{
    //�A�g���X�e�N�X�`���̃s�N�Z���z��
    std::vector<unsigned char> pixels(atlasWidth * atlasHeight,0);

    for (auto itr = fontMap.begin(); itr != fontMap.end(); itr++)
    {
        const uint32_t c = itr->first;
        const Rect& rect = itr->second.rect;

        if (!rect.rotated)
        {
            //������E�ɁA��s���ǂݎ���Ă���

            for (int i = rect.y + PADDING_PIXEL; i < rect.height - PADDING_PIXEL; i++)
            {
                for (int j = rect.x + PADDING_PIXEL; j < rect.width - PADDING_PIXEL; j++)
                {
                    const int location = (j - rect.x - 1) + (i - rect.y - 1) * rect.height;

                    const unsigned char black =
                        static_cast<unsigned char*>(bitmaps[c]->getPixelsData())[location];

                    pixels[i * atlasHeight + j] = black;
                }
            }
        }
        else
        {
            //������E�ɁA��s���ǂݎ���Ă���

            for (int i = rect.y + PADDING_PIXEL; i < rect.height - PADDING_PIXEL; i++)
            {
                for (int j = rect.x + PADDING_PIXEL; j < rect.width - PADDING_PIXEL; j++)
                {
                    const int location = (j - rect.x - 1) + (i - rect.y - 1) * rect.height;

                    const unsigned char black =
                        static_cast<unsigned char*>(bitmaps[c]->getPixelsData())[location];

                    pixels[i * atlasHeight + j] = black;
                }
            }
        }
    }

    //�A�g���X�e�N�X�`�����쐬
    atlasTexture = std::make_shared<ImageData>(atlasWidth, atlasHeight, 1, pixels.data());
    //gpu��ɉ摜��W�J
    VkFormat format = VK_FORMAT_R8_UNORM;
    VulkanBase::GetInstance()->createTexture(atlasTexture, format);
    //VkDescriptorSet�̍쐬
    createDescriptorSet();
}

//VkDescriptorSet�̍쐬
void FontManager::createDescriptorSet()
{
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = atlasTexture->getTexture()->view;
    imageInfo.sampler = atlasTexture->getTexture()->sampler;

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = atlasTexDescriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    VkDevice device = VulkanBase::GetInstance()->getDevice();

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

std::vector<CharFont>& FontManager::getCharFont(const std::string str)
{
    std::vector<CharFont> charFont;

    const char* begin = str.c_str();
    const char* end = begin + strlen(str.c_str());
    while (begin < end)
    {
        uint32_t code = utf8::next(begin, end);

        charFont.push_back(fontMap[code]);
    }

    return charFont;
}