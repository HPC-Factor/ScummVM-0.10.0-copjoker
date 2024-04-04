/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/parallaction/detection.cpp $
 * $Id: detection.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"

#include "base/plugins.h"

#include "common/advancedDetector.h"

#include "parallaction/parallaction.h"

namespace Parallaction {

struct PARALLACTIONGameDescription {
	Common::ADGameDescription desc;

	int gameType;
	uint32 features;
};

int Parallaction::getGameType() const { return _gameDescription->gameType; }
uint32 Parallaction::getFeatures() const { return _gameDescription->features; }
Common::Language Parallaction::getLanguage() const { return _gameDescription->desc.language; }
Common::Platform Parallaction::getPlatform() const { return _gameDescription->desc.platform; }
}

static const PlainGameDescriptor parallactionGames[] = {
	{"parallaction", "Parallaction engine game"},
	{"nippon", "Nippon Safes Inc."},
	{0, 0}
};

namespace Parallaction {

static const PARALLACTIONGameDescription gameDescriptions[] = {
	{
		{
			"nippon",
			"",
			{
				{ "disk1",	0, "610363308258e926dbabd5a9e7bb769f", 1060142},
				{ "disk2",	0, "bfdd7bcfbc226f4acf3f67fa9efa2826", 907205},
				{ "disk3",	0, "eec08180240888d76e3cfe3e183d5d5d", 1030721},
				{ "disk4",	0, "5bffddc7db226bdaa7dd3e10e5a15e68", 1151403},
				{ "en", 	0, "65cbfa81eafe308621184796ed116700", 399360},
				{ "fr", 	0, "ac20c743ea10f2cb4491f76c5644582c", 410624},
				{ "ge", 	0, "50916bfa34aee1380e0e959b37eceb5a", 410624},
				{ "it", 	0, "89964aef04d2c53a615ee8983caf2775", 410624},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GType_Nippon,
		0,
	},


	{
		{
			"nippon",
			"",
			{
				{ "disk0",	0, "6fed2e18a6bfe5e8bb49144fcc95fd11", 624640},
				{ "fr", 	0, "72f04be4320dfac719431419ec2b9a0d", 12778},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			Common::ADGF_DEMO
		},
		GType_Nippon,
		GF_DEMO,
	},


	{ AD_TABLE_END_MARKER, 0, 0 }
};

}

static const Common::ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Parallaction::gameDescriptions,
	// Size of that superset structure
	sizeof(Parallaction::PARALLACTIONGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	parallactionGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"parallaction",
	// List of files for file-based fallback detection (optional)
	0,
	// Fallback callback
	0,
	// Flags
	Common::kADFlagAugmentPreferredTarget
};

ADVANCED_DETECTOR_DEFINE_PLUGIN(PARALLACTION, Parallaction::Parallaction, detectionParams);

REGISTER_PLUGIN(PARALLACTION, "Parallaction engine", "Nippon Safes Inc. (C) Dynabyte");


namespace Parallaction {

bool Parallaction::detectGame() {
	_gameDescription = (const PARALLACTIONGameDescription *)Common::AdvancedDetector::detectBestMatchingGame(detectionParams);
	return (_gameDescription != 0);
}

} // End of namespace Parallaction
