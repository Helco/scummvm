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
#include "topgun/detection.h"
#include "topgun/console.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/palette.h"

namespace TopGun {

TopGunEngine *g_engine;

TopGunEngine::TopGunEngine(OSystem *syst, const TopGunGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc),
	_randomSource("Topgun"),
	_debug(true),
	_script(new Script(this)) {
	g_engine = this;

	gDebugLevel = kSuperVerbose;
	DebugMan.enableAllDebugChannels();
}

TopGunEngine::~TopGunEngine() {
	for (size_t i = 0; i < _scenes.size(); i++)
		delete _scenes[i];
	_scenes.clear();
	g_engine = nullptr;
}

uint32 TopGunEngine::getFeatures() const {
	return _gameDescription->_baseDescription.flags;
}

Common::String TopGunEngine::getGameId() const {
	return _gameDescription->_baseDescription.gameId;
}

Common::Error TopGunEngine::run() {
	initGraphics(800, 600);
	_spriteCtx.reset(new SpriteContext(this));

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	// TODO: Load Cursors
	// TODO: Init Audio
	// TODO: Init Sprite
	// TODO: Set MessageProc, MovieProc, ServiceProc

	if (!sceneIn("tama.bin"))
		return Common::kUnknownError;

	// Simple event handling loop
	byte pal[256 * 3] = { 0 };
	Common::Event e;
	int offset = 0;

	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
		}

		// Cycle through a simple palette
		++offset;
		for (int i = 0; i < 256; ++i)
			pal[i * 3 + 1] = (i + offset) % 256;
		g_system->getPaletteManager()->setPalette(pal, 0, 256);

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

Common::Error TopGunEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

bool TopGunEngine::sceneIn(const Common::String &name) {
	debugC(kInfo, kDebugRuntime, "SceneIn: %s", name.c_str());

	_spriteCtx->setCursor(SpriteContext::kSystemBusyCursor);

	_resFile.reset(new ResourceFile());
	if (!_resFile->load(name))
		return false;

	_resources.resize(_resFile->_totalResources);
	Common::fill(_resources.begin(), _resources.end(), nullptr);
	_scenes.push_back(new Scene(this, name));

	_spriteCtx->setPaletteFromResourceFile();
	_script->runEntry();

	return true;
}

bool TopGunEngine::isResourceLoaded(uint32 index) const {
	return _resources[index] != nullptr;
}

ResourceType TopGunEngine::getResourceType(uint32 index) const {
	return _resFile->_resources[index]._type;
}

SharedPtr<IResource> TopGunEngine::loadResource(uint32 index, ResourceType expectedType) {
	const auto actualType = getResourceType(index);
	if (actualType != expectedType)
		error("Attempted to load resource %index, expecting a type of %d, but it was %d", index, expectedType, actualType);
	if (isResourceLoaded(index))
		return _resources[index];
	debugCN(kTrace, kDebugResource, "Loading resource %d\n", index);

	auto resourceLocation = _resFile->_resources[index];
	switch (resourceLocation._type) {
	case ResourceType::kScript:
		_resources[index].reset(new ScriptResource(index));
		break;

	default:
		error("Unsupported resource type: %d", resourceLocation._type);
	}

	auto data = _resFile->loadResource(index);
	if (!_resources[index]->load(Common::move(data)))
		error("Could not load resource %d (type %d)", index, resourceLocation._type);

	return _resources[index];
}

void TopGunEngine::freeResource(uint32 index) {
	_resources[index] = nullptr;
}

} // End of namespace Topgun
