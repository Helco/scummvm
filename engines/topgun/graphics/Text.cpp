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

#include "topgun/topgun.h"
#include "topgun/graphics/Text.h"

namespace TopGun {

constexpr bool operator&(TextAlignment a, TextAlignment b) {
	return static_cast<byte>(a) & static_cast<byte>(b);
}

Text::Text(SpriteContext *spriteCtx, uint32 index) :
	ISurfaceResource(kResourceType, index),
	_spriteCtx(spriteCtx) {
}

bool Text::load(Common::Array<byte> &&data) {
	auto engine = _spriteCtx->getEngine();
	assert(engine->getResourceFile()->_architecture == Architecture::kBits32);
	auto stream = Common::MemorySeekableReadWriteStream(data.data(), data.size());

	_color = stream.readUint32LE();
	const auto fontNameIndex = stream.readUint32LE();
	const auto valueIndex = stream.readUint32LE();
	auto fontHeight = stream.readSint32LE();
	_pos.x = stream.readSint32LE();
	_pos.y = stream.readSint32LE();
	_size.x = stream.readSint32LE();
	_size.y = stream.readSint32LE();
	_alignment = (TextAlignment)stream.readUint32LE();
	stream.skip(1);
	const auto isFontHeightIndirect = stream.readByte() != 0;
	_wrap = stream.readByte() != 0;
	_isPassword = stream.readByte() != 0;

	_value = valueIndex == 0 ? "" : engine->getScript()->getString(valueIndex);
	fontHeight = engine->getScript()->evalValue(fontHeight, isFontHeightIndirect);
	const auto fontName = engine->getScript()->getString(fontNameIndex);
	_font = _spriteCtx->loadFont(fontName, fontHeight);

	renderText();
	return !stream.err();
}

void Text::renderText() {
	Common::String value;
	if (_value.empty())
		value = " ";
	else if (_isPassword) {
		value = _value;
		const auto size = _showLastPasswordCharacter && value.size()
			? value.size() - 1
			: value.size();
		memset(value.begin(), '*', size);
	}
	else
		value = _value;

	Common::Array<Common::String> lines;
	int32 width = _size.x;
	if (_wrap) {
		const auto maxWidth = _size.x == 0 ? INT32_MAX : _size.x;
		width = _font->wordWrapText(value, maxWidth, lines);
	}
	if (width == 0)
		width = _font->getStringWidth(value);
	int32 height = _size.y ? _size.y : lines.size() * _font->getFontHeight();

	_surface.reset(new Graphics::ManagedSurface(width, height));
	_surface->clear();
	for (size_t i = 0; i < lines.size(); i++)
		_font->drawString(_surface.get(), lines[i], 0, i * _font->getFontHeight(), width, _color);

	_offset = _pos;
	if (_alignment & TextAlignment::kRight)
		_offset.x += (width - 1) / 2;
	else if (_alignment & TextAlignment::kLeft)
		_offset.x -= width / 2;
	if (_alignment & TextAlignment::kBottom)
		_offset.y += (height - 1) / 2;
	else if (_alignment & TextAlignment::kTop)
		_offset.y -= height / 2;

	for (auto &sprite : _referencingSprites) {
		// TODO: Implement
	}
}

}
