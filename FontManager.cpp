#include"FontManager.h"

FontManager::FontManager()
{
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


    // 3. �����T�C�Y��ݒ�
    error = FT_Set_Char_Size(face, 0,
        16 * 54, // ���ƍ���
        300, 300);  // �����A�����𑜓x*/

    slot = face->glyph;
}

const CharFont& FontManager::getCharFont(const char c)
{
    if (fontMap.find(c) != fontMap.end())
    {
        return fontMap[c];
    }

    FT_Load_Glyph(face, FT_Get_Char_Index(face, c), FT_LOAD_RENDER);
    
    const FT_Bitmap& bitmap = slot->bitmap;

    std::shared_ptr<ImageData> fontImage =
        std::make_shared<ImageData>(bitmap);
}