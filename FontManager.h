#pragma once

#include<iostream>
#include<unordered_map>

#include"ft2build.h"
#include FT_FREETYPE_H

#include"Material.h"

struct CharFont
{
	std::shared_ptr<ImageData> fontImage;
};

class FontManager
{
private:

	static FontManager* instance;

	FontManager();

	FT_Library library;
	FT_Face face;
	FT_GlyphSlot slot;

	const std::string fontPath = "NotoSansJP-Regular.ttf";

	std::unordered_map<char, CharFont> fontMap;

public:

	static FontManager* GeInstance()
	{
		if (!instance)
		{
			instance = new FontManager();
		}

		return instance;
	}

	//ˆê‚Â‚Ì•¶Žš‚ÌƒtƒHƒ“ƒg‰æ‘œ‚ðŽæ“¾
	const CharFont& getCharFont(const char c);
};