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

static const u32char_type_t *strchr(const u32char_type_t *str, u32char_type_t chr) {
	assert(str != nullptr);
	while (*str && *str != chr)
		str++;
	return *str ? str : nullptr;
}

static size_t strlen(const u32char_type_t *str) {
	assert(str != nullptr);
	size_t len = 0;
	while (*str)
		len++;
	return len;
}

static const u32char_type_t *skipSpace(const u32char_type_t *text) {
	assert(text != nullptr);
	while (*text == ' ')
		text++;
	return text;
}

static const u32char_type_t *skipNonSpace(const u32char_type_t *text) {
	assert(text != nullptr);
	const u32char_type_t *space = strchr(text, ' ');
	if (space == nullptr)
		space = text + strlen(text);
	return space;
}

Text::Text(Point pos, FontKind font, const char *text, TextFlags flags)
	: _flags(flags)
	, _font(font) {
	assert(text != nullptr);
	setText(text, flags);
	setPos(pos);
}

Text::Text(Point pos, FontKind font, const U32String &text, TextFlags flags)
	: _flags(flags)
	, _font(font) {
	setText(text, flags);
	setPos(pos);
}

void Text::setText(const char *text, TextFlags flags) {
	setText(U32String(text), flags);
}

void Text::setText(const U32String &fullText, TextFlags flags) {
	_flags = flags;
	const FontInfo fontInfo = g_engine->assets().font(_font);

	size_t i = 0;
	for (; i < kDebugTextSize - 1 && i < fullText.size(); i++)
		_debugText[i] = fullText[i] < 0x80 ? fullText[i] : '#';
	_debugText[i] = 0;

	const auto *text = skipSpace(fullText.begin());
	if (flags & kTextWrapLines) {
		const auto *textEnd = text + strlen(text);
		while (textEnd - text > kMaxLineLength) {
			const auto *lineEnd = text + kMaxLineLength;
			while (lineEnd > text && *lineEnd != ' ')
				lineEnd--;

			if (lineEnd == text) // one massive word
				lineEnd = skipNonSpace(lineEnd);
			else { // the regular case
				while (lineEnd > text && *lineEnd == ' ')
					lineEnd--;
				lineEnd++;
			}
			_lines.emplace_back(g_engine->renderer().createText(fontInfo, text, lineEnd));
			text = skipSpace(lineEnd);
		}

		if (text < textEnd)
			_lines.emplace_back(g_engine->renderer().createText(fontInfo, text, textEnd));
	} else
		_lines.emplace_back(g_engine->renderer().createText(fontInfo, text));

	Point size(0, _lines.size() * kTextLineHeight);
	for (const auto &line : _lines)
		size.x = MAX(size.x, line->size().x);
	this->size() = size;
}

void Text::setPos(Common::Point pos) {
	const FontInfo fontInfo = g_engine->assets().font(_font);
	if (_flags & kTextAlignCenter) {
		pos.x -= size().x / 2;
		pos.y -= kTextOffsetY + (fontInfo._fgFont->getFontHeight() - kTextLineHeight) * _lines.size();
	}
	if (_flags & kTextMoveIntoScreen) {
		if (pos.x + size().x + kTextMargin > kScreenWidth)
			pos.x = kScreenWidth - kTextMargin - size().x;
		if (pos.x < kTextMargin)
			pos.x = kTextMargin;
	}
	this->pos() = pos;
}

void Text::debugPrint() {
	g_engine->getDebugger()->debugPrintf("Text \"%s\"\n", _debugText);
}

void Text::render() {
	if (!active())
		return;
	for (uint i = 0; i < _lines.size(); i++)
		g_engine->renderer().text(_lines[i].get(), getLineRenderPos(i));
}

bool Text::checkClick(Common::Point screenPos) const {
	for (uint i = 0; i < _lines.size(); i++) {
		Point relPos = screenPos - getLineRenderPos(i);
		if (_lines[i]->alphaCheck(relPos))
			return true;
	}
	return false;
}

Point Text::getLineRenderPos(uint lineI) const {
	assert(lineI < _lines.size());
	Point pos = this->pos();
	if (_flags & kTextAlignCenter)
		pos.x += size().x / 2 - _lines[lineI]->size().x / 2;
	pos.y += lineI * kTextLineHeight;
	return pos;
}

void Text::setColor(FontKind font) {
	const FontInfo fontInfo = g_engine->assets().font(font);
	for (auto &line : _lines)
		line->setColor(fontInfo);
}

}
