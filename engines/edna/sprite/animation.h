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

#ifndef EDNA_ANIMATION_H
#define EDNA_ANIMATION_H

#include "edna/graphics.h"
#include "edna/util.h"

namespace Edna {

class CharacterAnimationSet;

class Animation {
public:
    Animation(AnimationId id);

	inline bool isValid() const { return _range.isValid(); }
    inline const char *name() const { return _name; }
    inline AnimationRange range() const { return _range; }
    inline TextureSpan textures() const {
		if (!isValid())
			return TextureSpan();
        assert(_textures != nullptr);
        return {
            _textures->data() + _range._startFrame,
            _range._endFrame - _range._startFrame + 1
        };
    }

private:
    friend class CharacterAnimationSet;
    Animation();
    void loadTextures(AnimationId id, TextureArray &targetTextures);

    // only used if not part of a character animation set
    TextureArray *_textures = nullptr;
    TextureArray _ownTextures;
    AnimationRange _range;
    const char *_name = nullptr;
};

class CharacterAnimationSet {
public:
    CharacterAnimationSet(CharAnimSetId charAnimSetId);

	inline CharAnimSetId id() const { return _id; }
    inline const char *name() const { return _name; }
	AnimationRange get(ActionModeId actionMode, Direction direction);
    inline TextureSpan textures() {
		return { _textures.data(), _textures.size() };
    }

private:
	static constexpr uint32 kMaxActionMode = 8;
	const CharAnimSetId _id;
    TextureArray _textures;
	bool _hasActionMode[kMaxActionMode] = { false };
    Animation
		_left[kMaxActionMode],
		_right[kMaxActionMode],
		_up[kMaxActionMode],
		_down[kMaxActionMode];
    const char *_name = nullptr;
};

}

#endif // EDNA_ANIMATION_H
