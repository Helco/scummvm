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

#ifndef EDNA_ASSETCACHE_H
#define EDNA_ASSETCACHE_H

#include "util.h"

#include "graphics/managed_surface.h"

namespace Common {
    class File;
}

namespace Edna {

class ITexture;

// there are some assets (like fonts or normal player textures) that are used 
// so much that we want to keep them in memory, at least across room changes

class AssetCache {
public:
    AssetCache();

	void pushExitCursor() const;
    const FontInfo font(FontKind kind) const;

    // We delay freeing textures until the room is loaded by keeping another
    // shared pointer to them. After loading everything that was only used
    // in the last is freed.
    Common::SharedPtr<ITexture> texture(const Common::String &filename);
    void finishNextLoad();

private:
	Graphics::ManagedSurface _standardCursor, _exitCursor;

	Common::ScopedPtr<Graphics::Font>
		_fgFont14, _bgFont14,
		_fgFont16, _bgFont16,
		_fgFont18, _bgFont18;

    Common::HashMap<Common::String, Common::SharedPtr<ITexture>> _textureCaches[2];
    int _nextTextureCache = 0;
};

}

#endif // EDNA_ASSETCACHE_H
