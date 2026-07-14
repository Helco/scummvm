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

namespace Edna {

struct EdnaGameDescription;
class DB;
class IRenderer;

class EdnaEngine : public Engine {
protected:
	Common::Error run() override;
public:
	EdnaEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~EdnaEngine() override;

	inline IRenderer &renderer() { assert(_renderer != nullptr); return *_renderer; }
	inline DB &db() { assert(_db != nullptr); return *_db; }

	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
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
	void initGraphics();

	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	Common::ScopedPtr<IRenderer> _renderer;
	Common::ScopedPtr<DB> _db;
};

extern EdnaEngine *g_engine;
#define SHOULD_QUIT ::Edna::g_engine->shouldQuit()

} // End of namespace Edna

#endif // EDNA_H
