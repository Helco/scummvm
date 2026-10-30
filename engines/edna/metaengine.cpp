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

#include "edna/detection.h"
#include "edna/edna.h"
#include "edna/game/game.h"
#include "edna/metaengine.h"

#include "common/translation.h"
#include "graphics/scaler.h"

namespace Edna {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"original_menus",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace Edna

const char *EdnaMetaEngine::getName() const {
	return "edna";
}

const ADExtraGuiOptionsMap *EdnaMetaEngine::getAdvancedExtraGuiOptions() const {
	return Edna::optionsList;
}

void EdnaMetaEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	if (Edna::g_engine == nullptr || !Edna::g_engine->getSavegameThumbnail(thumb)) {
		// even if we cannot create a thumbnail, callers expect thumb to be created
		// we create it in size and format that ScummVM later expects to avoid conversions
		thumb.create(kThumbnailWidth, kThumbnailHeight2, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
		thumb.fillRect(thumb.getRect(), 0);
	}
}

Common::Error EdnaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Edna::EdnaEngine(syst, desc);
	return Common::kNoError;
}

bool EdnaMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

#if PLUGIN_ENABLED_DYNAMIC(EDNA)
REGISTER_PLUGIN_DYNAMIC(EDNA, PLUGIN_TYPE_ENGINE, EdnaMetaEngine);
#else
REGISTER_PLUGIN_STATIC(EDNA, PLUGIN_TYPE_ENGINE, EdnaMetaEngine);
#endif
