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

CharacterAnimationSet::CharacterAnimationSet(CharAnimSetId charAnimSetId, ActionModeId actionModeId) {
    const auto dbSet = g_engine->db().characterAnimationSet(charAnimSetId, actionModeId);
    _name = dbSet._name;
    _left.loadTextures(dbSet._left, _textures);
    _right.loadTextures(dbSet._right, _textures);
    _forward.loadTextures(dbSet._forward, _textures);
    _back.loadTextures(dbSet._back, _textures);
}
 
}
