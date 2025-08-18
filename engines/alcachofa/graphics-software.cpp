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
 * GNU General Public License for more dchrome://vivaldi-webui/startpage?section=Speed-dials&background-color=#2e2f37etails.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "alcachofa/graphics.h"
#include "alcachofa/detection.h"

#include "common/system.h"
#include "engines/util.h"
#include "graphics/blit.h"

using namespace Common;
using namespace Math;
using namespace Graphics;

namespace Alcachofa {

/* Summary of blend mode implementations:
 *
 * For textured quads:
 *   - TexA will almost always be either 0 or 1 but not guaranteed
 *   - Only Alpha is compatible with Graphics::BLEND_NORMAL, but only without trilinear filtering
 *
 * For colored quad (so assume that TexRGBA=[1, 1, 1, 1]):
 *   - TintGrey = [1, 1, 1] * TintA
 *   - AdditiveAlpha is solid fill with TintGrey
 *   - Additive is BLEND_ADDITIVE with colorMod = TintGrey
 *   - Multiply is *not compatible*, but perhaps also not necessary...
 *   - Alpha is solid white fill
 *   - Tinted is solid fill with TintGrey * TintA
 *
 * For anything other a custom implementation has to be used.
 */

class SoftwareTexture : public ITexture {
	// enough to get a 2k texture down to thumbnail size
	static constexpr int kMaxMipmaps = 4;
	static constexpr int32 kMinTextureSize = 16;
public:
	SoftwareTexture(int32 w, int32 h, bool withMipmaps)
		: ITexture({ (int16)w, (int16)h })
		, _withMipmaps(withMipmaps) {

		// We do not want too many mipmaps
		// and we do not want mipmaps too small to be useful
		int32 minSize = MIN(w, h);
		for (_maxMipmapCount = 0;
			minSize >= kMinTextureSize && _maxMipmapCount <= kMaxMipmaps;
			_maxMipmapCount++)
			minSize /= 2;
	}

	void update() override {
		_mipmapCount = 0;
	}

	int getMipmapIndex(Point targetSize, float lodBias) const {
		float scaleFactor = targetSize.x > targetSize.y
			? _surface.w / (float)targetSize.x
			: _surface.h / (float)targetSize.y;
		int lod = log2f(scaleFactor) + lodBias; // truncate to always downscale
		lod = CLIP(lod, 0, _maxMipmapCount);
		return lod;
	}

	const ManagedSurface &getMipmap(int lod) {
		if (lod <= 0)
			return _surface;
		lod--;
		for (; _mipmapCount <= lod; _mipmapCount++) {
			const ManagedSurface &previous = _mipmapCount == 0
				? _surface
				: _mipmaps[_mipmapCount - 1];
			ManagedSurface &next = _mipmaps[_mipmapCount];

			if (next.w != previous.w / 2)
				next.create(previous.w / 2, previous.h / 2, BlendBlit::getSupportedPixelFormat());
			scaleBlitBilinear(
				(byte *)next.getPixels(), (const byte *)previous.getPixels(),
				next.pitch, previous.pitch,
				next.w, next.h, previous.w, previous.h,
				next.format);
		}
		return _mipmaps[lod];
	}

private:
	ManagedSurface _mipmaps[kMaxMipmaps];
	int _mipmapCount = 0, ///< the count we have already initialized
		_maxMipmapCount; ///< the max mipmap count for *this* texture size
	bool _withMipmaps;
};

static Point roundVectorToPoint(Vector2d v) {
	return Point(
		(int16)(v.getX() + 0.5f),
		(int16)(v.getY() + 0.5f));
}

// to reduce code duplication for the quad grid iteration
struct QuadCellIterator {
	QuadCellIterator(
		int component,
		Rect fullBounds,
		Point texSize,
		Vector2d fullTexMin,
		Vector2d fullTexMax) {
		// For easier iteration we normalize the tex coords so that
		//   - texMin <= texMax
		//   - texMin >= 0
		_fullTexMin = fullTexMin(component, 0);
		_fullTexMax = fullTexMax(component, 0);

		// flip for non-even starting cells (as part of the mirror wrapping)
		_firstFlip = ((int)_fullTexMin) % 2 != 0;

		// flip again if we would need to iterate backwards
		if (_fullTexMin > _fullTexMax) {
			_firstFlip = !_firstFlip;
			_fullTexMin *= -1;
			_fullTexMax *= -1;
		}

		// shift to positive
		float fullTexMinInt;
		float fullTexMinFract = fabsf(modff(_fullTexMin, &fullTexMinInt));
		if (_fullTexMin < 0) {
			// fullTexMinInt will negative here
			_fullTexMin -= fullTexMinInt;
			_fullTexMax -= fullTexMinInt;
		}

		// calculate pixel layout
		_dstFullStart = component == 0 ? fullBounds.left : fullBounds.top;
		int16 fullSize = component == 0 ? fullBounds.width() : fullBounds.height();
		float fullFloatSize = fullSize / (_fullTexMax - _fullTexMin);
		assert(isfinite(fullFloatSize) && fullFloatSize >= 1 && fullFloatSize < INT16_MAX);
		_dstCellSize = (int16)(fullFloatSize);

		_srcCellSize = component == 0 ? texSize.x : texSize.y;
		_srcFullStart = (int16)(_srcCellSize * fullTexMinFract);

		// setup at the first cell
		_tex = _fullTexMax;
		next();
	}

	bool next() {
		bool hasNext;
		if (_tex >= _fullTexMax) {
			hasNext = false;
			_tex = _fullTexMin;
			_srcStart = _srcFullStart;
			_dstStart = _dstFullStart;
			_flipped = _firstFlip;
		} else {
			hasNext = true;
			_srcStart = _srcEnd;
			_dstStart = _dstEnd;
			_flipped = !_flipped;
		}

		float texStart = _tex;
		_tex = MIN(floorf(_tex + 1.0f), _fullTexMax);
		_srcEnd = _srcStart + (int16)(_srcCellSize * (_tex - texStart));
		_dstEnd = _dstStart + (int16)(_dstCellSize * (_tex - texStart));
		return hasNext;
	}

	float _tex = 0; ///< main iteration cursor
	int16 _dstStart = 0, _dstEnd = 0; ///< position on the screen
	int16 _srcStart = 0, _srcEnd = 0; ///< pixel position within (full-size) texture
	bool _flipped = false;

	int16 _dstFullStart, _dstCellSize;
	int16 _srcFullStart, _srcCellSize;
	float _fullTexMin, _fullTexMax;
	bool _firstFlip;
};

class SoftwareRenderer : public IRenderer {
public:
	SoftwareRenderer(Point resolution)
		: _resolution(resolution) {
	}

	void begin() override {
		assert(_screen == nullptr);
		_isFirstDrawCommand = true;
		_blendMode = {};
		_lodBias = 0;
		_otherOutput = nullptr;
		_outputScale = Vector2d(1.0f, 1.0f);
	}

	void end() override {
		if (_otherOutput == nullptr) {
			assert(_screen != nullptr);
			g_system->unlockScreen();
		}
		_otherOutput = nullptr;
		_screen = nullptr;
	}

	void setOutput(Surface &output) override {
		assert(_isFirstDrawCommand);
		assert(output.format == BlendBlit::getSupportedPixelFormat());
		_otherOutput = &output;
		_outputScale.setX((float)output.w / _resolution.x);
		_outputScale.setY((float)output.h / _resolution.y);
	}

	bool hasOutput() const override {
		return _otherOutput != nullptr;
	}

	void setBlendMode(BlendMode blendMode) override {
		_blendMode = blendMode;
	}

	void setLodBias(float lodBias) override {
		_lodBias = lodBias;
	}

	void setTexture(ITexture *texture) override {
		_texture = dynamic_cast<SoftwareTexture *>(texture);
		assert(texture == nullptr || _texture != nullptr); // only pass SoftwareTexture instances please
	}

	ScopedPtr<ITexture> createTexture(int32 w, int32 h, bool withMipmaps) override {
		assert(w >= 0 && h >= 0);
		return ScopedPtr<ITexture>(new SoftwareTexture(w, h, withMipmaps));
	}

	void quad(
		Vector2d topLeft,
		Vector2d size,
		Color color,
		Angle rotation,
		Vector2d texMin,
		Vector2d texMax) {
		if (rotation != Angle {} && !_didWarnAboutRotation) {
			// I do not know about any section of the supported games where
			// rotation can actually happen. As this would add another layer
			// of complication to this software renderer, it is delayed until
			// we know that rotation is necessary.
			warning(
				"Rotation is not implemented in the software renderer.\n"
				"If you see this message, please open a bug report describing where this warning happened.");
			_didWarnAboutRotation = true;
		}
		checkFirstDrawCommand();

		topLeft = topLeft * _outputScale;
		size = size * _outputScale;
		Rect bounds(
			roundVectorToPoint(topLeft),
			roundVectorToPoint(topLeft + size));
		if (bounds.isEmpty())
			return;
		else if (_texture == nullptr)
			coloredQuad(bounds, color);
		else if (texMin == Vector2d() && texMax == Vector2d(1, 1)) {
			// float equality is fine here, if it was calculated it is a special effects graphic
			// requiring mirror wrapping anyway
			int lod = _texture->getMipmapIndex({ bounds.width(), bounds.height() }, _lodBias);
			texturedQuad(bounds, Rect({}, _texture->size()), color, lod, false, false);
		} else {
			// at this point we might have a grid of mirror-wrapped
			QuadCellIterator x(0, bounds, _texture->size(), texMin, texMax);
			QuadCellIterator y(1, bounds, _texture->size(), texMin, texMax);
			int lod = _texture->getMipmapIndex({ x._dstCellSize, y._dstCellSize }, _lodBias);
			do {
				do {
					Rect target(x._dstStart, y._dstStart, x._dstEnd - 1, y._dstEnd - 1);
					Rect subRect(x._srcStart, y._srcStart, x._srcEnd - 1, y._srcEnd - 1);
					texturedQuad(target, subRect, color, lod, x._flipped, y._flipped);
				} while (x.next());
			} while (y.next());
		}
	}

	static void proportionalClip(
		int16 &target,
		int16 &subRect,
		int16 targetSize,
		int16 subRectSize,
		int16 maxTarget) {
		if (target >= 0 && target <= maxTarget)
			return;
		int16 prevTarget = target;
		target = CLIP(target, (int16)0, maxTarget);
		subRect += (target - prevTarget) * subRectSize / targetSize;
	}

	void texturedQuad(Rect target, Rect subRect, Color color, int lod, bool horizontalFlip, bool verticalFlip) {
		// at this point we have a single subrect of the texture to be (scale-)blit onto the output
		// we still need to clip it against output for BlendBlit though
		auto *output = activeOutput();
		proportionalClip(target.left, subRect.left, target.width(), subRect.width(), output->w);
		proportionalClip(target.right, subRect.right, target.width(), subRect.width(), output->w);
		proportionalClip(target.bottom, subRect.bottom, target.height(), subRect.height(), output->h);
		proportionalClip(target.top, subRect.top, target.height(), subRect.height(), output->h);
		assert(Rect({}, output->w, output->h).contains(target));
		if (target.isEmpty())
			return;

		// and adjust for the chosen LOD
		const auto &input = _texture->getMipmap(lod);
		subRect.left >>= lod;
		subRect.top >>= lod;
		subRect.right >>= lod;
		subRect.bottom >>= lod;
		assert(Rect({}, input.w, input.h).contains(subRect));
		if (subRect.isEmpty())
			return;
		
		// and calculate the scaling
		int scaleX = BlendBlit::getScaleFactor(subRect.width(), target.width());
		int scaleY = BlendBlit::getScaleFactor(subRect.height(), target.height());
		uint flipping =
			(horizontalFlip ? FLIP_H : FLIP_NONE) |
			(verticalFlip ? FLIP_V : FLIP_NONE);

		// and actually draw it
		BlendBlit::blit(
			(byte *)output->getPixels(),
			(byte *)input.getBasePtr(subRect.left, subRect.top),
			output->pitch,
			input.pitch,
			target.left, target.top,
			target.width(), target.height(),
			//subRect.width(), subRect.height(),
			scaleX, scaleY,
			0, 0,
			0xFFFFFFFF,
			flipping,
			BLEND_NORMAL,
			ALPHA_FULL);
	}

	void coloredQuad(Rect bounds, Color color) {
		switch (_blendMode) {
		case BlendMode::AdditiveAlpha:
			solidQuadFill(bounds, color.a, color.a, color.a);
			break;
		case BlendMode::Alpha:
			solidQuadFill(bounds, 255, 255, 255);
			break;
		case BlendMode::Tinted:
			solidQuadFill(
				bounds,
				(uint8)((color.r * color.a) >> 8),
				(uint8)((color.g * color.a) >> 8),
				(uint8)((color.b * color.a) >> 8));
			break;
		case BlendMode::Additive: {
			Surface *output = activeOutput();
			uint32 rawColor = output->format.ARGBToColor(255, color.a, color.a, color.a);
			BlendBlit::fill(
				(byte *)output->getBasePtr(bounds.left, bounds.top),
				output->pitch,
				bounds.width(),
				bounds.height(),
				rawColor,
				BLEND_ADDITIVE);
			break;
		}
		case BlendMode::Multiply:
			if (_didWarnAboutColoredMultiply)
				return;
			_didWarnAboutColoredMultiply = true;
			warning(
				"Colored quads with multiply blending is not implemented in the software renderer.\n"
				"If you see this message, please open a bug report describing where this warning happened.");
			break;
		default:
			assert(false && "Invalid blend mode");
			break;
		}
	}

	void solidQuadFill(Rect bounds, byte r, byte g, byte b) {
		uint32 rawColor = activeOutput()->format.ARGBToColor(255, r, g, b);
		activeOutput()->fillRect(bounds, rawColor);
	}

private:
	Surface *activeOutput() {
		return _otherOutput == nullptr ? _screen : _otherOutput;
	}

	void checkFirstDrawCommand() {
		// We delay locking and clearing the screen. It is much easier for the game
		// to switch to a framebuffer before and not locking the screen at all
		if (!_isFirstDrawCommand)
			return;
		_isFirstDrawCommand = false;
		_screen = g_system->lockScreen();
		_screen->fillRect(Rect({}, _screen->w, _screen->h), 0);
	}

	Point _resolution;
	BlendMode _blendMode = {};
	float _lodBias = 0;
	SoftwareTexture *_texture = nullptr;
	Surface *_otherOutput = nullptr;
	Surface *_screen = nullptr;
	Vector2d _outputScale = Vector2d(1.0f, 1.0f);
	bool _isFirstDrawCommand = false;
	bool _didWarnAboutRotation = false;
	bool _didWarnAboutColoredMultiply = false;
};

IRenderer *IRenderer::createSoftwareRenderer(Point resolution) {
	auto format = BlendBlit::getSupportedPixelFormat();
	initGraphics(resolution.x, resolution.y, &format);
	return new SoftwareRenderer(resolution);
}

}
