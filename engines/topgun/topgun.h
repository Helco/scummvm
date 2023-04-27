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

#ifndef TOPGUN_H
#define TOPGUN_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"
#include "graphics/cursorman.h"

#include "topgun/detection.h"
#include "topgun/ResourceFile.h"
#include "topgun/Resource.h"
#include "topgun/Scene.h"
#include "topgun/Savestate.h"
#include "topgun/script/Script.h"
#include "topgun/script/IPlugin.h"
#include "topgun/graphics/SpriteContext.h"

using Common::ScopedPtr;
using Common::SharedPtr;
using Common::WeakPtr;
using Common::Array;

namespace TopGun {

struct TopgunGameDescription;

class TopGunEngine : public Engine {
private:
	const TopGunGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
protected:
	Common::Error run() override;
public:
	TopGunEngine(OSystem *syst, const TopGunGameDescription *gameDesc);
	~TopGunEngine() override;

	uint32 getFeatures() const;

	Common::String getGameId() const;
	inline const TopGunGameDescription* getGameDesc() const {
		return _gameDescription;
	}

	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently() override {
		return true;
	}
	bool canSaveGameStateCurrently() override {
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

	inline SpriteContext *getSpriteCtx() {
		return _spriteCtx.get();
	}
	inline Script *getScript() {
		return _script.get();
	}
	inline ResourceFile *getResourceFile() {
		return _resFile.get();
	}
	inline Scene *getScene() {
		return _scenes.back();
	}
	inline Savestate *getSavestate() {
		return _savestate.get();
	}
	inline IPlugin *getLoadedPlugin(uint32 index) {
		return _plugins[index];
	}
	inline SharedPtr<Sprite> getTopMostSprite() {
		return _topMostSpriteIndex == 0
			? nullptr
			: _resources[_topMostSpriteIndex].dynamicCast<Sprite>();
	}

private:
	void loadPlugins();
	IPlugin *loadPlugin(const Common::String &name); // defined in plugins/loadPlugin.cpp

public:
	bool sceneIn(const Common::String &name);

	bool isResourceLoaded(uint32 index) const;
	ResourceType getResourceType(uint32 index) const;
	void freeResource(uint32 index);
	SharedPtr<IResource> loadResource(uint32 index, ResourceType expectedType);
	template<class TResource>
	inline SharedPtr<TResource> loadResource(uint32 index) {
		return loadResource(index, TResource::kResourceType).dynamicCast<TResource>();
	}

	void setTopMostSprite(Sprite *sprite);

private:
	bool _debug;
	ScopedPtr<ResourceFile> _resFile;
	ScopedPtr<SpriteContext> _spriteCtx;
	ScopedPtr<Script> _script;
	ScopedPtr<Savestate> _savestate;
	// FIXME: This array would be nicer with a moving push_back or even an emplace method, discuss with core team
	Array<Scene*> _scenes;
	Array<SharedPtr<IResource>> _resources;
	Array<IPlugin *> _plugins;

	uint32 _topMostSpriteIndex;
};

extern TopGunEngine *g_engine;
#define SHOULD_QUIT ::TopGun::g_engine->shouldQuit()

} // End of namespace Topgun

#endif // TOPGUN_H
