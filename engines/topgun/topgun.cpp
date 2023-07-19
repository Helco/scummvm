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
#include "topgun/graphics/Text.h"
#include "topgun/graphics/Cell.h"
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
	_script(new Script(this)),
	_savestate(new Savestate())  {
	g_engine = this;

	gDebugLevel = kVerbose;
	DebugMan.enableAllDebugChannels();
}

TopGunEngine::~TopGunEngine() {
	for (auto scene : _scenes)
		delete scene;
	for (auto plugin : _plugins)
		delete plugin;
	_resources.clear(); // Group objects will need access to the _resources array during deconstruction, so we clear it manually first
	g_engine = nullptr;
}

uint32 TopGunEngine::getFeatures() const {
	return _gameDescription->_baseDescription.flags;
}

Common::String TopGunEngine::getGameId() const {
	return _gameDescription->_baseDescription.gameId;
}

Common::Error TopGunEngine::run() {
	CursorMan.showMouse(true);
	initGraphics(800, 600);
	_spriteCtx.reset(new SpriteContext(this));

	setDebugger(new Console(this));

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	// TODO: Init Audio
	// TODO: Set MessageProc, MovieProc, ServiceProc

	if (!sceneIn("tama.bin"))
		return Common::kUnknownError;

	Common::Event e;
	while (!shouldQuit()) {
		getDebugger()->onFrame();

		while (g_system->getEventManager()->pollEvent(e)) {
			switch (e.type) {
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				switch ((TopGunEvent)e.customType) {
				case TopGunEvent::kClearTopMostSprite:
					if (_clearTopMostSpriteScript) {
						debugCN(kTrace, kDebugScript, "Running clear-top-most-sprite-script %d\n", _clearTopMostSpriteScript);
						_script->runMessage(_clearTopMostSpriteScript);
						_clearTopMostSpriteScript = 0;
					}
					setTopMostSprite(nullptr);
					break;
				case TopGunEvent::kChangeScene:
					handleChangeScene();
					break;
				}
				break;
			case Common::EVENT_KEYDOWN:
				_script->onKeyDown(e.kbd);
				break;
			case Common::EVENT_KEYUP:
				_script->onKeyUp(e.kbd);
				break;
			}
		}

		_spriteCtx->animate();
		// TODO: Call plugins with update function
		// TODO: Call no-input-script
		_script->updateTimers();
		// TODO: Update native timers
		// TODO: Update Hit detect triggers
		// TODO: Update movies
		_script->runMessageQueue();

		_spriteCtx->render();
		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

void TopGunEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
	_script->handleEnginePause(pause);
	_spriteCtx->handleEnginePause(pause);
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
	loadPlugins();

	_lastSceneIndex = _curSceneIndex;
	for (_curSceneIndex = 0; _curSceneIndex < _scenes.size(); _curSceneIndex++) {
		if (!_scenes[_curSceneIndex]->getName().compareToIgnoreCase(name))
			break;
	}
	if (_curSceneIndex == _scenes.size())
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
	if (actualType != expectedType && expectedType != ResourceType::kInvalid)
		error("Attempted to load resource %i, expecting a type of %d, but it was %d", index, expectedType, actualType);
	_script->getDebugger()->onResource(false, index);
	if (isResourceLoaded(index))
		return _resources[index];

	auto resourceLocation = _resFile->_resources[index];
	switch (resourceLocation._type) {
	case ResourceType::kBitmap:
		debugCN(kTrace, kDebugResource, "Loading bitmap %d\n", index);
		_resources[index].reset(new Bitmap(index));
		break;
	case ResourceType::kWave:
		debugCN(kTrace, kDebugResource, "Loading wave %d\n", index);
		_resources[index].reset(new RawDataResource(ResourceType::kWave, index));
		warning("stub wave resource");
		break;
	case ResourceType::kCell:
		_resources[index].reset(new Cell(index));
		break;
	case ResourceType::kGroup:
		debugCN(kTrace, kDebugResource, "Loading resource group %d\n", index);
		_resources[index].reset(new Group(index));
		break;
	case ResourceType::kQueue:
		debugCN(kTrace, kDebugResource, "Loading queue %d\n", index);
		_resources[index].reset(new SpriteMessageQueue(index));
		break;
	case ResourceType::kScript:
		_resources[index].reset(new ScriptResource(index));
		break;
	case ResourceType::kSprite:
		debugCN(kTrace, kDebugResource, "Loading sprite %d\n", index);
		_resources[index] = _spriteCtx->createSprite(index);
		break;
	case ResourceType::kText:
		debugCN(kTrace, kDebugResource, "Loading text %d\n", index);
		_resources[index].reset(new Text(getSpriteCtx(), index));
		break;
	default:
		error("Unsupported resource type: %d", resourceLocation._type);
	}

	auto data = _resFile->loadResource(index);
	if (!_resources[index]->load(Common::move(data)))
		error("Could not load resource %d (type %d)", index, resourceLocation._type);
	_script->getDebugger()->onResource(true, index);

	return _resources[index];
}

void TopGunEngine::freeResource(uint32 index) {
	if (isResourceLoaded(index) && getResourceType(index) == ResourceType::kSprite)
		_spriteCtx->removeSprite(index);
	_resources[index] = nullptr;
}

void TopGunEngine::loadPlugins() {
	clearPlugins();
	_plugins.reserve(_resFile->_plugins.size());
	for (auto &pluginName : _resFile->_plugins)
		_plugins.push_back(loadPlugin(pluginName));
}

void TopGunEngine::clearPlugins() {
	for (auto plugin : _plugins)
		delete plugin;
	_plugins.clear();
}

void TopGunEngine::setTopMostSprite(Sprite *sprite) {
	if (_topMostSpriteIndex != 0)
		freeResource(_topMostSpriteIndex);

	_topMostSpriteIndex = sprite == nullptr ? 0 : sprite->getResourceIndex();
}

void TopGunEngine::postClearTopMostSprite(int32 script) {
	_clearTopMostSpriteScript = script;

	Common::Event ev;
	ev.type = Common::EVENT_CUSTOM_ENGINE_ACTION_START;
	ev.customType = (Common::CustomEventType)TopGunEvent::kClearTopMostSprite;
	g_system->getEventManager()->pushEvent(ev);
}

void TopGunEngine::updatePickedSprite() {
	warning("stub: updatePickedSprite");
}

void TopGunEngine::updatePickedSprite(int32 x, int32 y) {
	warning("stub: updatePickedSprite");
}

void TopGunEngine::leavePickedSprite() {
	if (!_pickedSprite)
		return;
	_script->postSpritePicked(_pickedSprite, false);
	_pickedSprite = 0;
}

size_t TopGunEngine::getClickRectIndex(Rect rect) {
	size_t i;
	for (i = 0; i < _clickRects.size(); i++) {
		if (_clickRects[i]._rect == rect)
			break;
	}
	if (i == _clickRects.size())
		_clickRects.push_back(ClickRect());
	return i;
}

void TopGunEngine::setClickRectScripts(uint32 scriptIndex) {
	if (!scriptIndex) {
		_clickRects.clear();
		return;
	}

	for (auto &clickRect : _clickRects) {
		clickRect._enabled = false;
		clickRect._scriptIndex = scriptIndex;
	}
}

void TopGunEngine::toggleClickRects(bool toggle) {
	for (auto &clickRect : _clickRects)
		clickRect._enabled = toggle;
}

void TopGunEngine::toggleClickRect(Rect rect, bool toggle) {
	size_t i = getClickRectIndex(rect);
	_clickRects[i]._enabled = toggle;
}

void TopGunEngine::setClickRect(Rect rect, uint32 scriptIndex, int32 scriptArg) {
	size_t i = getClickRectIndex(rect);
	_clickRects[i]._enabled = false;
	_clickRects[i]._rect = rect;
	_clickRects[i]._scriptIndex = scriptIndex;
	_clickRects[i]._scriptArg = scriptArg;
}

void TopGunEngine::removeClickRect(Rect rect) {
	size_t i = getClickRectIndex(rect);
	_clickRects.remove_at(i);
}

void TopGunEngine::postQuitScene() {
	if (_curSceneIndex == _lastSceneIndex) {
		debugCN(kInfo, kDebugRuntime, "Quit scene to quit game\n");
		g_engine->quitGame();
		return;
	}
	postChangeScene(_scenes[_lastSceneIndex]->getName());
}

void TopGunEngine::postChangeScene(const Common::String &name) {
	debugCN(kInfo, kDebugRuntime, "Post scene change to %s\n", name.c_str());
	_nextSceneName = name;

	Common::Event ev;
	ev.type = Common::EVENT_CUSTOM_ENGINE_ACTION_START;
	ev.customType = (Common::CustomEventType)TopGunEvent::kChangeScene;
	g_system->getEventManager()->pushEvent(ev);
}

void TopGunEngine::handleChangeScene() {
	_lastSceneIndex = _curSceneIndex;
	g_system->getEventManager()->getEventDispatcher()->clearEvents();
	resetCurrentScene();
	sceneIn(_nextSceneName);
}

void TopGunEngine::resetCurrentScene() {
	_resources.clear();
	_clickRects.clear();

	// TODO: clear movies, timers, hitdetects, probably browseevents

	_spriteCtx->resetScene();
	clearPlugins();
}

void TopGunEngine::printSceneStack() {
	auto debugger = getDebugger();
	for (auto i = _scenes.size(); i > 0; i--)
		debugger->debugPrintf("%s%s\n", i - 1 == _curSceneIndex ? "> " : "",  _scenes[i - 1]->getName().c_str());
}

} // End of namespace Topgun
