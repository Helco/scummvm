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

#include "edna/group/group.h"
#include "edna/sprite/object.h"

using namespace Common;

namespace Edna {

Group::Group(const char *name) : _name(name) { }

Group::~Group() { }

void Group::update() {
	if (!_active)
		return;

	Array<uint> inactive;
	for (uint i = 0; i < _sprites.size(); i++) {
		_sprites[i]->update();
		if (!_sprites[i]->active() && !_sprites[i]->immutable())
			inactive.push_back(i);
	}

	// reverse to keep indices stable
	for (uint i = 1; i <= inactive.size(); i++)
		_sprites.remove_at(inactive[inactive.size() - i]);
}

void Group::render() {
	if (!_active)
		return;
	for (const auto &sprite : _sprites)
		sprite->render();
}

void Group::debugRender() {
	for (const auto &sprite : _sprites)
		sprite->debugRender();
}

void Group::add(Sprite *sprite, DisposeAfterUse::Flag dispose) {
	assert(sprite != nullptr);
	assert(sprite->_parent == nullptr);
	assert(find_if(_sprites.begin(), _sprites.end(),
		[sprite](const DisposablePtr<Sprite> &ptr) { return ptr.get() == sprite; }) == _sprites.end());
	sprite->_parent = this;
	_sprites.emplace_back(sprite, dispose);
}

Sprite *Group::byId(uint32 id) const {
	auto it = find_if(_sprites.begin(), _sprites.end(),
		[id](const DisposablePtr<Sprite> &ptr) { return ptr->id() == id; });
	return it == _sprites.end() ? nullptr : it->get();
}

Sprite *Group::firstActive() const {
	auto it = find_if(_sprites.begin(), _sprites.end(),
		[](const DisposablePtr<Sprite> &ptr) { return ptr->active(); });
	return it == _sprites.end() ? nullptr : it->get();
}

Sprite *Group::checkClick(Point screenPos) const {
	for (uint i = _sprites.size(); i > 0; i--) {
		if (_sprites[i - 1]->checkClick(screenPos))
			return _sprites[i - 1].get();
	}
	return nullptr;
}

template<typename Iterator, typename Comp>
static void stable_sort(Iterator begin, Iterator end, Comp comp) {
	if (begin == end)
		return;
	Iterator innerEnd = end;
	for (Iterator outer = begin; outer != end; ++outer) {
		bool didSwap = false;
		Iterator first = begin, second = begin;
		for (++second; second != innerEnd; ++second) {
			if (!comp(*first, *second)) {
				SWAP(*first, *second);
				didSwap = true;
			}
			first = second;
		}
		if (!didSwap)
			return;
		innerEnd = first;
	}
}

void ObjectGroup::render() {
	stable_sort(_sprites.begin(), _sprites.end(), [](const DisposablePtr<Sprite> &aRaw, const DisposablePtr<Sprite> &bRaw) {
		const auto *a = dynamic_cast<const GameObject *>(aRaw.get());
		const auto *b = dynamic_cast<const GameObject *>(bRaw.get());
		assert(a != nullptr && b != nullptr);
		int aBase = a->basePosY(b->basePosX());
		int bBase = b->basePosY(a->basePosX());
		return aBase <= bBase;
	});
	Group::render();
}

Sprite *ObjectGroup::checkInteractableClick(Point screenPos) const {
	for (uint i = _sprites.size(); i > 0; i--) {
		const auto *interactable = dynamic_cast<const IInteractable *>(_sprites[i - 1].get());
		if (interactable != nullptr && _sprites[i - 1]->checkClick(screenPos))
			return _sprites[i - 1].get();
	}
	return nullptr;
}

}
