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

#ifndef TOPGUN_IRESOURCE_H
#define TOPGUN_IRESOURCE_H

#include "topgun/ResourceFile.h"

namespace TopGun {

class TopGunEngine;

class IResource {
public:
	IResource(ResourceType type, uint32 index);
	virtual ~IResource() = default;

	virtual bool load(Common::Array<byte> &&data) = 0;

	inline ResourceType getResourceType() const {
		return _type;
	}

	inline uint32 getResourceIndex() const {
		return _index;
	}

private:
	ResourceType _type;
	uint32 _index;
};

class RawDataResource : public IResource {
public:
	RawDataResource(ResourceType type, uint32 index);
	virtual ~RawDataResource() = default;

	virtual bool load(Common::Array<byte> &&data) override;

	inline Common::Array<byte> &getData() {
		return _data;
	}

private:
	Common::Array<byte> _data;
};

class ScriptResource : public RawDataResource {
public:
	static constexpr ResourceType kResourceType = ResourceType::kScript;

	ScriptResource(uint32 index);
};

}

#endif // TOPGUN_IRESOURCE_H
