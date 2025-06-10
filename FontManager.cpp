#include"VulkanBase.h"

#include"FontManager.h"

FontManager* FontManager::instance = nullptr;

FontManager::FontManager()
{
    texturePack = nullptr;

    // 1. libraryを初期化
    auto error = FT_Init_FreeType(&library);
    if (error)
    {
        throw std::runtime_error("font library initialize failed");
    }

    // 2. faceを作成
    error = FT_New_Face(library, fontPath.c_str(), 0, &face);
    if (error)
    {
        throw std::runtime_error("font face create failed");
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    slot = face->glyph;

    createAtlasTexture();
}

//フォント全体の高さを取得
const float FontManager::getFontHeight()
{
    return face->height / 64.0;
}

FontManager::~FontManager()
{
    VkDevice device = VulkanBase::GetInstance()->getDevice();

    atlasTexture->cleanUpVulkan(device);
}

//日本語のアトラステクスチャを作る
void FontManager::createAtlasTexture()
{
    //文字のビットマップを取得する
    loadJapaneseFile();

    //それらをくっつけてすべての文字のビットマップを
    //含むテクスチャを作る
    
    //アトラステクスチャの縦と横のサイズ
    int atlasWidth, atlasHeight;
    //作成したビットマップの数
    int characterNum = static_cast<int>(fontMap.size());

    //ビットマップの総面積を計算する
    int totalArea = 0;
    for (auto itr = fontMap.begin(); itr != fontMap.end(); itr++)
    {
        totalArea += itr->second.size.x * itr->second.size.y;
    }

    //おおよそのテクスチャサイズの一辺の大きさ
    int estimatedSize = static_cast<int>(std::sqrt(totalArea));

    //テクスチャのサイズを計算
    //なおテクスチャのサイズは2のべき乗にする
    int texSize = 1;
    while (texSize < estimatedSize)
    {
        texSize *= 2;
    }

    //アトラステクスチャのサイズを決定
    atlasWidth = texSize;
    atlasHeight = texSize;

    //テクスチャパッキングを開始する
    texturePack = std::make_unique<MaxRectPacking>(atlasWidth, atlasHeight);

    //テクスチャパックに挿入していく
    //パッキングに失敗した場合は、アトラステクスチャのサイズを大きくする
    bool packingSuccess = true;
    for (auto itr = fontMap.begin(); itr != fontMap.end(); itr++)
    {
        itr->second.rect = texturePack->insert(packingSuccess,itr->second.size.x, itr->second.size.y);
    }

    //テクスチャパッキングを終了する
    texturePack.reset();

    //文字一つずつのuvなどを設定していく
    for (auto itr = fontMap.begin(); itr != fontMap.end(); itr++)
    {
        itr->second.uvSet(atlasWidth, atlasHeight);
    }

    //すべての文字のビットマップをアトラステクスチャにまとめる
    packBitmapsToAtlas(atlasWidth, atlasHeight);
}

//日本語の一覧を文字列として読み込む
void FontManager::loadJapaneseFile()
{
    std::ifstream ifs(characterPath);

    if (!ifs)
    {
        throw std::runtime_error("failed load character file");
    }

    //すべての文字のビットマップを作成する
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

    //ファイルを閉じる
    ifs.close();
}

//ビットマップを作成
void FontManager::createBitmap(const uint32_t c)
{
    if (FT_Load_Char(face, c, FT_LOAD_RENDER))
    {
        return;
    }

    FT_GlyphSlot& glyph = face->glyph;

    //フォントのオフセットなどを設定
    CharFont font{};
    
    //パディングを含んだ、文字のビットマップのサイズ
    font.size = glm::vec2(glyph->bitmap.width + (2.0 * PADDING_PIXEL), glyph->bitmap.rows + (2.0 * PADDING_PIXEL));
    
    //文字をベースラインからどれくらいずらしてレンダリングすべきの値、y軸を反転させておく
    font.bearing = glm::vec2(glyph->metrics.horiBearingX / 64.0, -glyph->metrics.horiBearingY / 64.0);
    
    //次の文字への隙間
    font.advance = glyph->metrics.horiAdvance / 64.0;

    //改行時に下げるべきピクセル値
    font.fontHeight = face->size->metrics.height / 64.0;

    fontMap[c] = font;

    bitmaps[c] = std::make_shared<ImageData>(glyph->bitmap);
}

//すべての文字のビットマップをアトラステクスチャにまとめる
void FontManager::packBitmapsToAtlas(int atlasWidth, int atlasHeight)
{
    //アトラステクスチャのピクセル配列
    std::vector<unsigned char> pixels(atlasWidth * atlasHeight,0);

    for (auto itr = fontMap.begin(); itr != fontMap.end(); itr++)
    {
        const uint32_t c = itr->first;
        const Rect& rect = itr->second.rect;

        if (!rect.rotated)
        {
            //左から右に、一行ずつ読み取っていく

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
            //元のビットマップを反時計回りに90度回転して
            //アトラステクスチャにコピーする

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

    //アトラステクスチャを作成
    atlasTexture = std::make_shared<ImageData>(atlasWidth, atlasHeight, 1, pixels.data());
    //gpu上に画像を展開
    VkFormat format = VK_FORMAT_R8_UNORM;
    VulkanBase::GetInstance()->createTexture(atlasTexture, format);
    VulkanBase::GetInstance()->createFontDescriptorSet(atlasTexture, atlasTexDescriptorSet);
}



void FontManager::getCharFont(const std::vector<uint32_t>& utf8Codes, std::vector<CharFont>& charFonts)
{
    charFonts.clear();
    charFonts.resize(utf8Codes.size());

    //utf8Codesのunicodeから、対応する文字のグリフの情報をcharFontsに入れていく
    for (int i = 0; i < utf8Codes.size(); i++)
    {
        charFonts[i] = fontMap[utf8Codes[i]];
    }
}