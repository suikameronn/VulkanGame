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

//�t�H���g�S�̂̍������擾
const float FontManager::getFontHeight()
{
    return face->height / 64.0;
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
            uint32_t code = utf8::next(begin, end);//12360 12362

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
    CharFont font{};
    
    //�p�f�B���O���܂񂾁A�����̃r�b�g�}�b�v�̃T�C�Y
    font.size = glm::vec2(glyph->bitmap.width + (2.0 * PADDING_PIXEL), glyph->bitmap.rows + (2.0 * PADDING_PIXEL));
    
    //�������x�[�X���C������ǂꂭ�炢���炵�ă����_�����O���ׂ��̒l�Ay���𔽓]�����Ă���
    font.bearing = glm::vec2(glyph->metrics.horiBearingX / 64.0, -glyph->metrics.horiBearingY / 64.0);
    
    //���̕����ւ̌���
    font.advance = glyph->metrics.horiAdvance / 64.0;

    //���s���ɉ�����ׂ��s�N�Z���l
    font.fontHeight = face->size->metrics.height / 64.0;

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

            for (int i = rect.y + PADDING_PIXEL; i < rect.y + rect.height - PADDING_PIXEL; i++)
            {
                for (int j = rect.x + PADDING_PIXEL; j < rect.x + rect.width - PADDING_PIXEL; j++)
                {
                    const int location = (j - rect.x - PADDING_PIXEL) + (i - rect.y - PADDING_PIXEL) * (rect.width - 2 * PADDING_PIXEL);

                    const unsigned char black = bitmaps[c]->getAt(location);

                    pixels.at(i * atlasWidth + j) = black;
                }
            }
        }
        else
        {
            //���̃r�b�g�}�b�v�𔽎��v����90�x��]����
            //�A�g���X�e�N�X�`���ɃR�s�[����

            const std::shared_ptr<ImageData>& bitmap = bitmaps[c];
            for (int i = 0; i < bitmap->getHeight(); i++)
            {
                for (int j = 0; j < bitmap->getWidth(); j++)
                {
                    const unsigned char black = static_cast<unsigned char*>(bitmap->getPixelsData())[i * bitmap->getWidth() + j];

                    int rotateX = i;
                    int rotateY = bitmap->getWidth() - 1 - j;

                    int atlasRotateX = rect.x + PADDING_PIXEL + rotateX;
                    int atlasRotateY = rect.y + PADDING_PIXEL + rotateY;

                    const int location = atlasRotateY * atlasWidth + atlasRotateX;
                    pixels.at(location) = black;
                }
            }
        }
    }

    //�A�g���X�e�N�X�`�����쐬
    atlasTexture = std::make_shared<ImageData>(atlasWidth, atlasHeight, 1, pixels.data());
    //gpu��ɉ摜��W�J
    VkFormat format = VK_FORMAT_R8_UNORM;
    VulkanBase::GetInstance()->createTexture(atlasTexture, format);
    VulkanBase::GetInstance()->createFontDescriptorSet(atlasTexture, atlasTexDescriptorSet);
}



void FontManager::getCharFont(const std::vector<uint32_t>& utf8Codes, std::vector<CharFont>& charFonts)
{
    charFonts.clear();
    charFonts.resize(utf8Codes.size());

    //utf8Codes��unicode����A�Ή����镶���̃O���t�̏���charFonts�ɓ���Ă���
    for (int i = 0; i < utf8Codes.size(); i++)
    {
        charFonts[i] = fontMap[utf8Codes[i]];
    }
}