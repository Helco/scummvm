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

#include "alcachofa/alcachofa.h"
#include "graphics/framelimiter.h"
#include "alcachofa/detection.h"
#include "alcachofa/console.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "graphics/framelimiter.h"
#include "video/mpegps_decoder.h"

#include "rooms.h"
#include "script.h"

using namespace Math;

namespace Alcachofa {

AlcachofaEngine *g_engine;

AlcachofaEngine::AlcachofaEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Alcachofa") {
	g_engine = this;
}

AlcachofaEngine::~AlcachofaEngine() {
}

uint32 AlcachofaEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String AlcachofaEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error AlcachofaEngine::run() {
	g_system->showMouse(false);
	setDebugger(_console);
	_renderer.reset(IRenderer::createOpenGLRenderer(Common::Point(1024, 768)));
	_drawQueue.reset(new DrawQueue(_renderer.get()));
	_world.reset(new World());
	_script.reset(new Script());
	_player.reset(new Player());

	//_script->createProcess(MainCharacterKind::None, "Inicializar_Variables");
	//_player->changeRoom("MINA", true);
	_script->createProcess(MainCharacterKind::None, "CREDITOS_INICIALES");
	_scheduler.run();

	Common::Event e;
	Graphics::FrameLimiter limiter(g_system, 120);
	while (!shouldQuit()) {
		_input.nextFrame();
		while (g_system->getEventManager()->pollEvent(e)) {
			if (_input.handleEvent(e))
				continue;
		}

		_sounds.update();
		_renderer->begin();
		_drawQueue->clear();
		_camera.shake() = Vector2d();
		_player->preUpdate();
		_player->currentRoom()->update();
		_player->postUpdate();

		_renderer->end();

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

void AlcachofaEngine::playVideo(int32 videoId) {	
	Video::MPEGPSDecoder decoder;
	if (!decoder.loadFile(Common::Path(Common::String::format("Data/DATA%02d.BIN", videoId + 1))))
		error("Could not find video %d", videoId);
	auto texture = _renderer->createTexture(decoder.getWidth(), decoder.getHeight(), false);
	decoder.start();

	Common::Event e;
	while (!decoder.endOfVideo() && !shouldQuit()) {
		if (decoder.needsUpdate())
		{
			auto surface = decoder.decodeNextFrame();
			if (surface)
				texture->update(*surface);
			_renderer->begin();
			_renderer->setBlendMode(BlendMode::Alpha);
			_renderer->setLodBias(0.0f);
			_renderer->setTexture(texture.get());
			_renderer->quad({}, { (float)g_system->getWidth(), (float)g_system->getHeight() });
			_renderer->end();
		}

		_input.nextFrame();
		while (g_system->getEventManager()->pollEvent(e)) {
			if (_input.handleEvent(e))
				continue;
		}
		if (_input.wasAnyMouseReleased())
			break;

		g_system->updateScreen();
		g_system->delayMillis(decoder.getTimeToNextFrame() / 2);
	}
	decoder.stop();
}

Common::Error AlcachofaEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace Alcachofa
