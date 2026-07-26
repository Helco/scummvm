/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "edna/assetcache.h"
#include "edna/edna.h"
#include "edna/sprite/text.h"

#include "graphics/font.h"
#include "gui/debugger.h"

using namespace Common;

namespace Edna {

/// this is used instead of leading for some renderingxs
static constexpr const int16 kTextLineHeight = 20;
static constexpr const int16 kTextMargin = 10;
static constexpr const int16 kTextOffsetY = 26; ///< to be subtracted
static constexpr const int kMaxLineLength = 40;

static const char *skipSpace(const char *text) {
	assert(text != nullptr);
	while (*text == ' ')
		text++;
	return text;
}

static const char *skipNonSpace(const char *text) {
	assert(text != nullptr);
	const char *space = strchr(text, ' ');
	if (space == nullptr)
		space = text + strlen(text);
	return space;
}

Text::Text(Point pos, FontKind font, const char *text, TextFlags flags) {
	assert(text != nullptr);
	const FontInfo fontInfo = g_engine->assets().font(font);
	strncpy(_debugText, text, kDebugTextSize - 1);
	_debugText[Common::strnlen(_debugText, kDebugTextSize - 1)] = 0;

	if (flags & kTextWrapLines) {
		text = skipSpace(text);
		const char *textEnd = text + strlen(text);
		while (textEnd - text > kMaxLineLength) {
			const char *lineEnd = skipNonSpace(text + kMaxLineLength);			
			if (lineEnd - text <= kMaxLineLength) {
				_lines.emplace_back(g_engine->renderer().createText(fontInfo, text, lineEnd));
				text = skipSpace(lineEnd);
			} else {
				// find the end of the last word
				const char *wordEnd = lineEnd;
				while (wordEnd > text && *wordEnd != ' ')
					wordEnd--;
				while (wordEnd > text && *wordEnd == ' ')
					wordEnd--;

				if (text < wordEnd) {
					_lines.emplace_back(g_engine->renderer().createText(fontInfo, text, wordEnd));
					text = skipSpace(wordEnd);
				} else { // one massive word we cannot split
					_lines.emplace_back(g_engine->renderer().createText(fontInfo, text, lineEnd));
					text = skipSpace(lineEnd);
				}
			}
		}

		text = skipSpace(text);
		if (text < textEnd)
			_lines.emplace_back(g_engine->renderer().createText(fontInfo, text, textEnd));
	} else
		_lines.emplace_back(g_engine->renderer().createText(fontInfo, text));

	Point size(0, _lines.size() * kTextLineHeight);
	for (const auto &line : _lines)
		size.x = MAX(size.x, line->size().x);
	this->size() = size;

	if (flags & kTextMoveIntoScreen) {
		if (pos.x + size.x / 2 + kTextMargin > kScreenWidth)
			pos.x = kScreenWidth - kTextMargin - size.x / 2;
		if (pos.x - size.x / 2 < kTextMargin)
			pos.x = kTextMargin;
	}
	pos.x -= size.x / 2;
	pos.y -= kTextOffsetY + (fontInfo._fgFont->getFontHeight() - kTextLineHeight) * _lines.size();
	this->pos() = pos;
}

void Text::debugPrint() {
	g_engine->getDebugger()->debugPrintf("Text \"%s\"\n", _debugText);
}

void Text::render() {
	const int baseX = pos().x + size().x / 2;
	int cursorY = pos().y;
	for (auto &line : _lines) {
		int cursorX = baseX - line->size().x / 2;
		g_engine->renderer().text(line.get(), Point(cursorX, cursorY));
		cursorY += kTextLineHeight;
	}
}

void Text::setColor(FontKind font) {
	const FontInfo fontInfo = g_engine->assets().font(font);
	for (auto &line : _lines)
		line->setColor(fontInfo);
}

}
