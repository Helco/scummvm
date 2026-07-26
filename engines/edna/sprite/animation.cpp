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

#include "edna/edna.h"
#include "edna/db.h"
#include "edna/sprite/animation.h"

namespace Edna {

Animation::Animation() {}

Animation::Animation(AnimationId id)  {
    loadTextures(id, _ownTextures);
}

void Animation::loadTextures(AnimationId id, TextureArray &targetTextures) {
    _textures = &targetTextures;
	if (id == 0) {
		_name = "";
		return;
	}

    const auto dbAnim = g_engine->db().animation(id);   
    _name = dbAnim._name;
    _range._delay = dbAnim._duration;
    _range._loop = dbAnim._loop;
    _range._startFrame = _range._endFrame = _textures->size();

    const auto dbFrames = g_engine->db().animationFrames(id);
    uint32 totalFrames = 0;
    for (const auto &dbFrame : dbFrames)
        totalFrames += dbFrame._altDuration;
    _textures->reserve(_textures->size() + totalFrames);
    for (const auto &dbFrame : dbFrames) {
        auto texture = TexturePtr(g_engine->renderer().loadTexture(dbFrame._image));
        if (texture == nullptr)
            error("Could not load texture: %s", dbFrame._image);
        for (uint32 i = 0; i < dbFrame._altDuration; i++)
            _textures->push_back(texture);
        _range._endFrame += dbFrame._altDuration;
    }
	_range._endFrame--; // end frame is inclusive
	// ^ animationFrames errors if there are no frame so this cannot underflow
}

CharacterAnimationSet::CharacterAnimationSet(CharAnimSetId charAnimSetId) : _id(charAnimSetId) {
	if (charAnimSetId == 0) {
		_name = "empty";
		return;
	}

	// at least "waiting" has to exist, but we load the normal other ones as well already
	// "walking" has to be loaded first so the first texture for left is the base size of the character
	get(0, Direction::Left);
	get(1, Direction::Left);
	get(2, Direction::Left);
	get(3, Direction::Left); 
	if (_textures.empty())
		error("Could not load character animation set %u", charAnimSetId);
	if (_name == nullptr)
		_name = "empty";
}

AnimationRange CharacterAnimationSet::get(ActionModeId actionMode, Direction dir) {
	assert(actionMode < kMaxActionMode);
	if (!_hasActionMode[actionMode]) {
		_hasActionMode[actionMode];
		const auto dbSet = g_engine->db().characterAnimationSet(_id, actionMode, false);
		if (dbSet._id != _id || dbSet._actionMode != actionMode)
			return AnimationRange();
		if (_name != nullptr)
			_name = dbSet._name;

		_left[actionMode].loadTextures(dbSet._left, _textures);
		_right[actionMode].loadTextures(dbSet._right, _textures);
		_forward[actionMode].loadTextures(dbSet._forward, _textures);
		_back[actionMode].loadTextures(dbSet._back, _textures);
	}
	
	switch (dir) {
	case Direction::Left:
		return _left[actionMode].range();
	case Direction::Right:
		return _right[actionMode].range();
	case Direction::Up:
		return _forward[actionMode].range();
	case Direction::Down:
		return _back[actionMode].range();
	default:
		assert("Invalid direction for character animation set");
		return AnimationRange();
	}
}
 
}
