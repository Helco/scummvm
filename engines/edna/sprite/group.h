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

#ifndef EDNA_GROUP_H
#define EDNA_GROUP_H

#include "common/array.h"
#include "common/span.h"
#include "common/ptr.h"
#include "edna/util.h"

namespace Edna {

class Sprite;
class InteractableObject;

class Group {
public:
	Group(const char *name);
	virtual ~Group();

	inline const char *name() const { return _name; } ///< only for debugging
	inline bool &active() { return _active; }
	inline Common::Span<const Common::DisposablePtr<Sprite>> sprites() const {
		return { _sprites.data(), _sprites.size() };
	}

	virtual void update();
	virtual void render();
	virtual void debugRender();

	void add(Sprite *sprite, DisposeAfterUse::Flag dispose = DisposeAfterUse::YES);
	Sprite *byId(uint32 id) const;
	Sprite *firstActive() const;
	Sprite *checkClick(Common::Point screenPos) const;

protected:
	const char *const _name;
	Common::Array<Common::DisposablePtr<Sprite>> _sprites;
	bool _active = true;
};

class ObjectGroup : public Group {
public:
	using Group::Group;
	void render() override;
	InteractableObject *checkInteractableClick(Common::Point screenPos) const;
};

}

#endif // EDNA_GROUP_H
