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
#include "edna/graphics.h"

#include "engines/util.h"
#include "graphics/framelimiter.h"
#include "edna/detection.h"
#include "edna/console.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "common/compression/unzip.h"
#include "engines/util.h"

using namespace Common;

namespace Edna {

EdnaEngine *g_engine;

EdnaEngine::EdnaEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Edna") {
	g_engine = this;
}

EdnaEngine::~EdnaEngine() {
}

static void addArchive(const char *name) {
	Path path("data/");
	path.appendInPlace(name);
	Archive *archive = makeZipArchive(path);
	if (archive == nullptr)
		error("Could not find data archive: %s", name);
	SearchMan.add(name, archive);
}

Error EdnaEngine::run() {
	setDebugger(new Console());

	const char *language = getLanguageCode(_gameDescription->language);
	addArchive("audio.jar");
	addArchive("comments.jar");
	addArchive("visual.jar");
	addArchive((String("speech_") + language + ".jar").c_str());

	_db.reset(new DB(Path("script/").appendInPlace(language)));
	_renderer.reset(createSoftwareRenderer());
	initGraphics();

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	Common::Event e;
	Graphics::FrameLimiter limiter(g_system, 40); // this is the original framerate in 1.3.1 
	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
		}

		_renderer->begin();

		limiter.delayBeforeSwap();
		_renderer->end();
		limiter.startFrame();
	}

	return Common::kNoError;
}

void EdnaEngine::initGraphics() {
	
}

Error EdnaEngine::syncGame(Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return kNoError;
}

} // End of namespace Edna
