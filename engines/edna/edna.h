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

#ifndef EDNA_H
#define EDNA_H

#include "audio/mixer.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/ptr.h"
#include "engines/engine.h"
#include "engines/savestate.h"

#include "edna/detection.h"
#include "edna/util.h"

namespace Edna {

struct EdnaGameDescription;
class IRenderer;
class DB;
class GameBase;
class Console;

class EdnaEngine : public Engine {
protected:
	Common::Error run() override;
public:
	EdnaEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~EdnaEngine() override;

	inline RoomId &nextRoom() { return _nextRoom; }
	inline const char *language() const { return getLanguageCode(_gameDescription->language); }
	inline IRenderer &renderer() { assert(_renderer != nullptr); return *_renderer; }
	inline Console &console() { assert(_console != nullptr); return *_console; }
	inline DB &db() { assert(_db != nullptr); return *_db; }
	inline GameBase &game() { assert(_game != nullptr); return *_game; }

	inline uint32 getElapsed() const { return _timeElapsed; }
	inline float getElapsedF() const { return _timeElapsed / 1000.0f; }
	uint32 getMillis() const;
	void setMillis(uint32 newMillis);
	Audio::SoundHandle playMusic(const char *fileName, bool loop = true);
	void pauseEngineIntern(bool pause) override;

	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return false;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return false;
	}

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	}

private:
	GameBase *createRoom(RoomId roomId);

	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	Common::ScopedPtr<IRenderer> _renderer;
	Console *_console; // raw pointer because Engine deletes the console itself
	Common::ScopedPtr<DB> _db;
	Common::ScopedPtr<GameBase> _game;

	uint32 _timeNegOffset = 0, _timePosOffset = 0;
	uint32 _timeBeforePause = 0;
	uint32 _timeLastFrame = 0, _timeElapsed = 0;
	RoomId _nextRoom = 0;
};

extern EdnaEngine *g_engine;
#define SHOULD_QUIT ::Edna::g_engine->shouldQuit()

} // End of namespace Edna

#endif // EDNA_H
