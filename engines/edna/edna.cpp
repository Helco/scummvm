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

#include "edna/assetcache.h"
#include "edna/console.h"
#include "edna/edna.h"
#include "edna/detection.h"
#include "edna/db.h"
#include "edna/graphics.h"
#include "edna/game/ednagirl.h"
#include "edna/game/ednastd.h"
#include "edna/game/harvey.h"
#include "edna/game/intro.h"
#include "edna/game/scriptonclick.h"
#include "edna/input.h"
#include "edna/pathfinder.h"
#include "edna/translation.h"

#include "audio/decoders/vorbis.h"
#include "audio/audiostream.h"
#include "graphics/font.h"
#include "graphics/framelimiter.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "common/compression/unzip.h"

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
	setDebugger(_console = new Console());

	addArchive("audio.jar");
	addArchive("comments.jar");
	addArchive("visual.jar");
	addArchive((String("speech_") + language() + ".jar").c_str());

	Config::registerDefaults();
	_config.loadFromScummVM();
	_db.reset(new DB(Path("script/").appendInPlace(language())));
	_renderer.reset(createSoftwareRenderer());
	_input.reset(new Input());
	_assets.reset(new AssetCache());
	_translation.reset(new Translation(_gameDescription->language));
	_pathFinder.reset(new PathFinder());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot == -1)
		new Intro(_game, true);
	else
		(void)loadGameState(saveSlot);

	_transition = {};
	//_transition._room = 100101;
	//_transition._walkIn = { 260, 520 };
	_transition._room = 400101;
	_transition._walkIn = { 400, 500 };
	_transition._walkInDir = Direction::Left;

	_timeLastFrame = getMillis();
	Common::Event e;
	Graphics::FrameLimiter limiter(g_system, 40); // this is the original framerate
	while (!shouldQuit()) {
		uint32 now = getMillis();
		_timeElapsed = now > _timeLastFrame ? now - _timeLastFrame : 1;
		_timeLastFrame = now;
		input().nextFrame();

		// change room before handling events as we publish events to be processed by the next room
		if (_transition.isPending()) {
			createRoom(_transition);
			assets().finishNextLoad();
			_transition = {};
		}

		while (g_system->getEventManager()->pollEvent(e)) {
			if (input().handleEvent(e))
				continue;
		}
		_game->update();

		_renderer->begin();
		_game->render();
		_game->debugRender();
		limiter.delayBeforeSwap();
		_renderer->end();
		limiter.startFrame();
	}

	return Common::kNoError;
}

void EdnaEngine::createRoom(const GameTransition &transition) {
	DB::Room room = db().room(transition._room);
	switch (room._gameMode) {
	case GameMode::ScriptOnClick:
		new ScriptOnClick(_game, transition);
		break;
	case GameMode::EdnaStd:
		new EdnaStd(_game, transition);
		break;
	case GameMode::EdnaGirl:
		new EdnaGirl(_game, transition);
		break;
	case GameMode::Harvey:
		new Harvey(_game, transition);
		break;
	default:
		error("Unimplemented game mode: %s", gameModeToString(room._gameMode));
	}
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

uint32 EdnaEngine::getMillis() const {
	// We modify system time in order to simplify timing calculations
	// and pauses (including pauses by in-game menus)
	return g_system->getMillis() - _timeNegOffset + _timePosOffset;
}

void EdnaEngine::setMillis(uint32 newMillis) {
	const uint32 sysMillis = g_system->getMillis();
	if (newMillis > sysMillis) {
		_timeNegOffset = 0;
		_timePosOffset = newMillis - sysMillis;
	} else {
		_timeNegOffset = sysMillis - newMillis;
		_timePosOffset = 0;
	}
}

void EdnaEngine::pauseEngineIntern(bool pause) {
	if (pause)
		_timeBeforePause = getMillis();
	else
		setMillis(_timeBeforePause);
}

Audio::SoundHandle EdnaEngine::playSpeech(const char *fileName) {
	assert(fileName != nullptr);
	File *file = new File();
	if (!file->open(fileName) && !file->open(Path(String(fileName) + ".ogg"))) {
		warning("Could not open speech: %s", fileName);
		delete file;
		return {};
	}
	auto *fileStream = Audio::makeVorbisStream(file, DisposeAfterUse::YES);
	if (fileStream == nullptr) {
		warning("Could not decode speech: %s", fileName);
		return {};
	}
	Audio::SoundHandle handle;
	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &handle, fileStream);
	return handle;
}

Audio::SoundHandle EdnaEngine::playMusic(const char *fileName, bool loop) {
	assert(fileName != nullptr);
	if (_mixer->isSoundHandleActive(_musicHandle)) {
		if (_lastMusic == fileName)
			return _musicHandle; // the same music is already playing
		_mixer->stopHandle(_musicHandle);
	}

	File *file = new File();
	if (!file->open(Path(String(fileName) + ".ogg"))) {
		delete file;
		return {};
	}
	auto *fileStream = Audio::makeVorbisStream(file, DisposeAfterUse::YES);
	Audio::AudioStream *playStream = fileStream;
	if (loop)
		playStream = Audio::makeLoopingAudioStream(fileStream, 0);
	g_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, playStream);
	_lastMusic = fileName;
	return _musicHandle;
}

void EdnaEngine::stopMusic() {
	_mixer->stopHandle(_musicHandle);
	_musicHandle = {};
	_lastMusic.clear();
}

void Config::registerDefaults() {
	Config c;
	ConfMan.registerDefault("subtitles", c._subtitles);
	ConfMan.registerDefault("speech_mute", !c._speech);
	ConfMan.registerDefault("music_mute", !c._music);
	ConfMan.registerDefault("talkspeed", c._subtitleSpeed);
}

void Config::loadFromScummVM() {
	_subtitles = ConfMan.getBool("subtitles");
	_speech = !ConfMan.getBool("speech_mute");
	_music = !ConfMan.getBool("music_mute");
	_subtitleSpeed = (byte)CLIP(ConfMan.getInt("talkspeed"), 0, 255);
}

void Config::saveToScummVM() {
	ConfMan.setBool("subtitles", _subtitles);
	ConfMan.setBool("speech_mute", !_speech);
	ConfMan.setBool("music_mute", !_music);
	ConfMan.setInt("talkspeed", _subtitleSpeed);
	ConfMan.flushToDisk();
}

} // End of namespace Edna
