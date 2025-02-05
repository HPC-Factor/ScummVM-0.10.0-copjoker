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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/scumm/detection.cpp $
 * $Id: detection.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "common/stdafx.h"

#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/fs.h"
#include "common/list.h"
#include "common/md5.h"

#include "scumm/detection.h"
#include "scumm/detection_tables.h"
#include "scumm/scumm.h"
#include "scumm/intern.h"
#include "scumm/he/intern_he.h"

namespace Scumm {

enum {
	// We only compute the MD5 of the first megabyte of our data files.
	kMD5FileSizeLimit = 1024 * 1024
};

#pragma mark -
#pragma mark --- Miscellaneous ---
#pragma mark -

static const char *findDescriptionFromGameID(const char *gameid) {
	const PlainGameDescriptor *g = gameDescriptions;
	while (g->gameid) {
		if (!scumm_stricmp(g->gameid, gameid)) {
			return g->description;
		}
		g++;
	}
	error("Unknown gameid '%s' encountered in findDescriptionFromGameID", gameid);
}

static int compareMD5Table(const void *a, const void *b) {
	const char *key = (const char *)a;
	const MD5Table *elem = (const MD5Table *)b;
	return strcmp(key, elem->md5);
}

static const MD5Table *findInMD5Table(const char *md5) {
#ifdef PALMOS_68K
	uint32 arraySize = MemPtrSize((void *)md5table) / sizeof(MD5Table) - 1;
#else
	uint32 arraySize = ARRAYSIZE(md5table) - 1;
#endif
	return (const MD5Table *)bsearch(md5, md5table, arraySize, sizeof(MD5Table), compareMD5Table);
}

Common::String ScummEngine::generateFilename(const int room) const {
	const int diskNumber = (room > 0) ? _res->roomno[rtRoom][room] : 0;
	char buf[128];

	if (_game.version == 4) {
		if (room == 0 || room >= 900) {
			snprintf(buf, sizeof(buf), "%03d.lfl", room);
		} else {
			snprintf(buf, sizeof(buf), "disk%02d.lec", diskNumber);
		}
	} else {
		char id = 0;

		switch (_filenamePattern.genMethod) {
		case kGenDiskNum:
			snprintf(buf, sizeof(buf), _filenamePattern.pattern, diskNumber);
			break;
	
		case kGenRoomNum:
			snprintf(buf, sizeof(buf), _filenamePattern.pattern, room);
			break;

		case kGenHEMac:
		case kGenHEMacNoParens:
		case kGenHEPC:
			if (room < 0) {
				id = '0' - room;
			} else if (_game.heversion >= 98) {
				int disk = 0;
				if (_heV7DiskOffsets)
					disk = _heV7DiskOffsets[room];
		
				switch (disk) {
				case 2:
					id = 'b';
					snprintf(buf, sizeof(buf), "%s.(b)", _filenamePattern.pattern);
					break;
				case 1:
					id = 'a';
					snprintf(buf, sizeof(buf), "%s.(a)", _filenamePattern.pattern);
					break;
				default:
					id = '0';
					snprintf(buf, sizeof(buf), "%s.he0", _filenamePattern.pattern);
				}
			} else if (_game.heversion >= 70) {
				id = (room == 0) ? '0' : '1';
			} else {
				id = diskNumber + '0';
			}
			
			if (_filenamePattern.genMethod == kGenHEPC) {
				// For HE >= 98, we already called snprintf above.
				if (_game.heversion < 98 || room < 0)
					snprintf(buf, sizeof(buf), "%s.he%c", _filenamePattern.pattern, id);
			} else {
				if (id == '3') { // special case for cursors
					// For mac they're stored in game binary
					strncpy(buf, _filenamePattern.pattern, sizeof(buf));
				} else {
					if (_filenamePattern.genMethod == kGenHEMac)
						snprintf(buf, sizeof(buf), "%s (%c)", _filenamePattern.pattern, id);
					else
						snprintf(buf, sizeof(buf), "%s %c", _filenamePattern.pattern, id);
				}
			}

			break;

		case kGenUnchanged:
			strncpy(buf, _filenamePattern.pattern, sizeof(buf));
			break;

		default:
			error("generateFilename: Unsupported genMethod");
		}
	}

	return buf;
}

static Common::String generateFilenameForDetection(const char *pattern, FilenameGenMethod genMethod) {
	char buf[128];

	switch (genMethod) {
	case kGenDiskNum:
	case kGenRoomNum:
		snprintf(buf, sizeof(buf), pattern, 0);
		break;

	case kGenHEPC:
		snprintf(buf, sizeof(buf), "%s.he0", pattern);
		break;

	case kGenHEMac:
		snprintf(buf, sizeof(buf), "%s (0)", pattern);
		break;

	case kGenHEMacNoParens:
		snprintf(buf, sizeof(buf), "%s 0", pattern);
		break;

	case kGenUnchanged:
		strncpy(buf, pattern, sizeof(buf));
		break;

	default:
		error("generateFilenameForDetection: Unsupported genMethod");
	}

	return buf;
}

struct DetectorDesc {
	FilesystemNode node;
	Common::String md5;
	const MD5Table *md5Entry;	// Entry of the md5 table corresponding to this file, if any.
};

typedef Common::HashMap<Common::String, DetectorDesc, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> DescMap;

static bool testGame(const GameSettings *g, const DescMap &fileMD5Map, const Common::String &file);


// Search for a node with the given "name", inside fslist. Ignores case
// when performing the matching. The first match is returned, so if you 
// search for "resource" and two nodes "RESOURE and "resource" are present,
// the first match is used.
static bool searchFSNode(const FSList &fslist, const Common::String &name, FilesystemNode &result) {
	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (!scumm_stricmp(file->name().c_str(), name.c_str())) {
			result = *file;
			return true;
		}
	}
	return false;
}

// The following function tries to detect the language for COMI and DIG
static Common::Language detectLanguage(const FSList &fslist, byte id) {
	assert(id == GID_CMI || id == GID_DIG);

	// Check for LANGUAGE.BND (Dig) resp. LANGUAGE.TAB (CMI).
	// These are usually inside the "RESOURCE" subdirectory.
	// If found, we match based on the file size (should we
	// ever determine that this is insufficient, we can still
	// switch to MD5 based detection).
	const char *filename = (id == GID_CMI) ? "LANGUAGE.TAB" : "LANGUAGE.BND";
	Common::File tmp;
	FilesystemNode langFile;
	if (!searchFSNode(fslist, filename, langFile) || !tmp.open(langFile)) {
		// try loading in RESOURCE sub dir...
		FilesystemNode resDir;
		FSList tmpList;
		if (searchFSNode(fslist, "RESOURCE", resDir)
			&& resDir.isDirectory()
			&& resDir.listDir(tmpList, FilesystemNode::kListFilesOnly)
			&& searchFSNode(tmpList, filename, langFile)) {
			tmp.open(langFile);
		}
	}
	if (tmp.isOpen()) {
		uint size = tmp.size();
		if (id == GID_CMI) {
			switch (size) {
			case 439080:	// 2daf3db71d23d99d19fc9a544fcf6431
				return Common::EN_ANY;
			case 493252:	// 5d59594b24f3f1332e7d7e17455ed533
				return Common::DE_DEU;
			case 461746:	// 35bbe0e4d573b318b7b2092c331fd1fa
				return Common::FR_FRA;
			case 443439:	// 4689d013f67aabd7c35f4fd7c4b4ad69
				return Common::IT_ITA;
			case 440586:	// 5a1d0f4fa00917bdbfe035a72a6bba9d
				return Common::PT_BRA;
			case 449787:	// 64f3fe479d45b52902cf88145c41d172
				return Common::ES_ESP;
			}
		} else {
			switch (size) {
			case 248627:	// 1fd585ac849d57305878c77b2f6c74ff
				return Common::DE_DEU;
			case 257460:	// 04cf6a6ba6f57e517bc40eb81862cfb0
				return Common::FR_FRA;
			case 231402:	// 93d13fcede954c78e65435592182a4db 
				return Common::IT_ITA;
			case 228772:	// 5d9ad90d3a88ea012d25d61791895ebe
				return Common::PT_BRA;
			case 229884:	// d890074bc15c6135868403e73c5f4f36
				return Common::ES_ESP;
			}
		}
	}
	
	return Common::UNK_LANG;
}


static void computeGameSettingsFromMD5(const FSList &fslist, const GameFilenamePattern *gfp, const MD5Table *md5Entry, DetectorResult &dr) {
	dr.language = md5Entry->language;
	dr.extra = md5Entry->extra;

	// Compute the precise game settings using gameVariantsTable.
	for (const GameSettings *g = gameVariantsTable; g->gameid; ++g) {
		if (g->gameid[0] == 0 || !scumm_stricmp(md5Entry->gameid, g->gameid)) {
			// The gameid either matches, or is empty. The latter indicates
			// a generic entry, currently used for some generic HE settings.
			if (g->variant == 0 || !scumm_stricmp(md5Entry->variant, g->variant)) {
				// Perfect match found, use it and stop the loop
				dr.game = *g;
				dr.game.gameid = md5Entry->gameid;

				// Set the platform value. The value from the MD5 record has
				// highest priority; if missing (i.e. set to unknown) we try
				// to use that from the filename pattern record instead.
				if (md5Entry->platform != Common::kPlatformUnknown) {
					dr.game.platform = md5Entry->platform;
				} else if (gfp->platform != Common::kPlatformUnknown) {
					dr.game.platform = gfp->platform;
				}
				
				// HACK: Special case to distinguish the V1 demo from the full version
				// (since they have identical MD5):
				if (dr.game.id == GID_MANIAC && !strcmp(gfp->pattern, "%02d.MAN")) {
					dr.extra = "V1 Demo";
				}

				// HACK: If 'Demo' occurs in the extra string, set the GF_DEMO flag,
				// required by some game demos (e.g. Dig, FT and COMI).
				if (dr.extra && strstr(dr.extra, "Demo")) {
					dr.game.features |= GF_DEMO;
				}
				
				// HACK: Detect COMI & Dig languages
				if (dr.language == UNK_LANG && (dr.game.id == GID_CMI || dr.game.id == GID_DIG)) {
					dr.language = detectLanguage(fslist, dr.game.id);
				}
				break;
			}
		}
	}
}

static void detectGames(const FSList &fslist, Common::List<DetectorResult> &results, const char *gameid) {
	DescMap fileMD5Map;
	DetectorResult dr;
	char md5str[32+1];
	
	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (!file->isDirectory()) {
			DetectorDesc d;
			d.node = *file;
			d.md5Entry = 0;
			fileMD5Map[file->name()] = d;
		}
	}

	// Iterate over all filename patterns.
	for (const GameFilenamePattern *gfp = gameFilenamesTable; gfp->gameid; ++gfp) {
		// If a gameid was specified, we only try to detect that specific game,
		// so we can just skip over everything with a differing gameid.
		if (gameid && scumm_stricmp(gameid, gfp->gameid))
			continue;
	
		// Generate the detectname corresponding to the gfp. If the file doesn't
		// exist in the directory we are looking at, we can skip to the next
		// one immediately.
		Common::String file(generateFilenameForDetection(gfp->pattern, gfp->genMethod));
		if (!fileMD5Map.contains(file))
			continue;

		// Reset the DetectorResult variable
		dr.fp.pattern = gfp->pattern;
		dr.fp.genMethod = gfp->genMethod;
		dr.game.gameid = 0;
		dr.language = gfp->language;
		dr.md5.clear();
		dr.extra = 0;
		
		//  ____            _     _ 
		// |  _ \ __ _ _ __| |_  / |
		// | |_) / _` | '__| __| | |
		// |  __/ (_| | |  | |_  | |
		// |_|   \__,_|_|   \__| |_|
		//
		// PART 1: Trying to find an exact match using MD5.
		//
		//
		// Background: We found a valid detection file. Check if its MD5
		// checksum occurs in our MD5 table. If it does, try to use that
		// to find an exact match.
		//
		// We only do that if the MD5 hadn't already been computed (since
		// we may look at some detection files multiple times).
		//
		DetectorDesc &d = fileMD5Map[file];
		if (d.md5.empty()) {
			if (Common::md5_file_string(d.node, md5str, kMD5FileSizeLimit)) {

				d.md5 = md5str;
				d.md5Entry = findInMD5Table(md5str);

				dr.md5 = d.md5;

				if (d.md5Entry) {
					// Exact match found. Compute the precise game settings.
					computeGameSettingsFromMD5(fslist, gfp, d.md5Entry, dr);

					// Sanity check: We *should* have found a matching gameid / variant at this point.
					// If not, then there's a bug in our data tables...
					assert(dr.game.gameid != 0);
					
					// Add it to the list of detected games
					results.push_back(dr);
				}
			}
		}
		
		// If an exact match for this file has already been found, don't bother 
		// looking at it anymore.
		if (d.md5Entry)
			continue;


		//  ____            _     ____  
		// |  _ \ __ _ _ __| |_  |___ \ *
		// | |_) / _` | '__| __|   __) |
		// |  __/ (_| | |  | |_   / __/ 
		// |_|   \__,_|_|   \__| |_____|
		//
		// PART 2: Fuzzy matching for files with unknown MD5.
		//
		
		
		// We loop over the game variants matching the gameid associated to
		// the gfp record. We then try to decide for each whether it could be
		// appropriate or not.
		dr.md5 = d.md5;
		for (const GameSettings *g = gameVariantsTable; g->gameid; ++g) {
			// Skip over entries with a different gameid.
			if (g->gameid[0] == 0 || scumm_stricmp(gfp->gameid, g->gameid))
				continue;

			dr.game = *g;
			dr.extra = g->variant; // FIXME: We (ab)use 'variant' for the 'extra' description for now.

			if (gfp->platform != Common::kPlatformUnknown)
				dr.game.platform = gfp->platform;


			// If a variant has been specified, use that!
			if (gfp->variant) {
				if (!scumm_stricmp(gfp->variant, g->variant)) {
					// perfect match found
					results.push_back(dr);
					break;
				}
				continue;
			}
			
			
			// Add the game/variant to the candidates list if it is consistent
			// with the file(s) we are seeing.
			if (testGame(g, fileMD5Map, file))
				results.push_back(dr);
		}
	}
}

static bool testGame(const GameSettings *g, const DescMap &fileMD5Map, const Common::String &file) {
	const DetectorDesc &d = fileMD5Map[file];

	// At this point, we know that the gameid matches, but no variant
	// was specified, yet there are multiple ones. So we try our best
	// to distinguish between the variants.
	// To do this, we take a close look at the detection file and
	// try to filter out some cases.

	Common::File tmp;
	if (!tmp.open(d.node)) {
		warning("SCUMM detectGames: failed to open '%s' for read access", d.node.path().c_str());
		return false;
	}
	
	if (file == "maniac1.d64" || file == "maniac1.dsk" || file == "zak1.d64") {
		// TODO
	} else if (file == "00.LFL") {
		// Used in V1, V2, V3 games.
		if (g->version > 3)
			return false;

		// Read a few bytes to narrow down the game.				
		byte buf[6];
		tmp.read(buf, 6);
		
		if (buf[0] == 0xbc && buf[1] == 0xb9) {
			// The NES version of MM
			if (g->id == GID_MANIAC && g->platform == Common::kPlatformNES) {
				// perfect match
				return true;
			}
		} else if ((buf[0] == 0xCE && buf[1] == 0xF5) || // PC
			(buf[0] == 0xCD && buf[1] == 0xFE)) {    // Commodore 64
			// Could be V0 or V1.
			// Candidates: maniac classic, zak classic
			
			if (g->version >= 2)
				return false;

			// Zak has 58.LFL, Maniac doesn't have it.
			const bool has58LFL = fileMD5Map.contains("58.LFL");
			if (g->id == GID_MANIAC && !has58LFL) {
			} else if (g->id == GID_ZAK && has58LFL) {
			} else
				return false;
		} else if (buf[0] == 0xFF && buf[1] == 0xFE) {
			// GF_OLD_BUNDLE: could be V2 or old V3.
			// Note that GF_OLD_BUNDLE is true if and only if GF_OLD256 is false.
			// Candidates: maniac enhanced, zak enhanced, indy3ega, loom

			if (g->version != 2 && g->version != 3  || (g->features & GF_OLD256))
				return false;

			/* We distinguish the games by the presence/absence of
			   certain files. In the following, '+' means the file
			   present, '-' means the file is absent.

			   maniac:    -58.LFL, -84.LFL,-86.LFL, -98.LFL

			   zak:       +58.LFL, -84.LFL,-86.LFL, -98.LFL
			   zakdemo:   +58.LFL, -84.LFL,-86.LFL, -98.LFL

			   loom:      +58.LFL, -84.LFL,+86.LFL, -98.LFL
			   loomdemo:  -58.LFL, +84.LFL,-86.LFL, -98.LFL

			   indy3:     +58.LFL, +84.LFL,+86.LFL, +98.LFL
			   indy3demo: -58.LFL, +84.LFL,-86.LFL, +98.LFL
			*/
			const bool has58LFL = fileMD5Map.contains("58.LFL");
			const bool has84LFL = fileMD5Map.contains("84.LFL");
			const bool has86LFL = fileMD5Map.contains("86.LFL");
			const bool has98LFL = fileMD5Map.contains("98.LFL");

			if (g->id == GID_INDY3         && has98LFL && has84LFL) {
			} else if (g->id == GID_ZAK    && !has98LFL && !has86LFL && !has84LFL && has58LFL) {
			} else if (g->id == GID_MANIAC && !has98LFL && !has86LFL && !has84LFL && !has58LFL) {
			} else if (g->id == GID_LOOM   && !has98LFL && (has86LFL != has84LFL)) {
			} else
				return false;
		} else if (buf[4] == '0' && buf[5] == 'R') {
			// newer V3 game
			// Candidates: indy3, indy3Towns, zakTowns, loomTowns

			if (g->version != 3 || !(g->features & GF_OLD256))
				return false;

			/*
			Considering that we know about *all* TOWNS versions, and
			know their MD5s, we could simply rely on this and if we find
			something which has an unknown MD5, assume that it is an (so
			far unknown) version of Indy3. However, there are also fan
			translations of the TOWNS versions, so we can't do that.

			But we could at least look at the resource headers to distinguish
			TOWNS versions from regular games:

			Indy3:
			_numGlobalObjects 1000
			_numRooms 99
			_numCostumes 129
			_numScripts 139
			_numSounds 84

			Indy3Towns, ZakTowns, ZakLoom demo:
			_numGlobalObjects 1000
			_numRooms 99
			_numCostumes 199
			_numScripts 199
			_numSounds 199

			Assuming that all the town variants look like the latter, we can
			do the check like this:
			  if (numScripts == 139)
				assume Indy3
			  else if (numScripts == 199)
				assume towns game
			  else
				unknown, do not accept it
			*/
			
			// We now try to exclude various possibilities by the presence of certain
			// LFL files. Note that we only exclude something based on the *presence*
			// of a LFL file here; compared to checking for the absence of files, this
			// has the advantage that we are less likely to accidentally exclude demos
			// (which, after all, are usually missing many LFL files present in the
			// full version of the game).
			
			// No version of Indy3 has 05.LFL but MM, Loom and Zak all have it
			if (g->id == GID_INDY3 && fileMD5Map.contains("05.LFL"))
				return false;

			// All versions of Indy3 have 93.LFL, but no other game
			if (g->id != GID_INDY3 && fileMD5Map.contains("93.LFL"))
				return false;

			// No version of Loom has 48.LFL
			if (g->id == GID_LOOM && fileMD5Map.contains("48.LFL"))
				return false;

			// No version of Zak has 60.LFL, but most (non-demo) versions of Indy3 have it
			if (g->id == GID_ZAK && fileMD5Map.contains("60.LFL"))
				return false;

			// All versions of Indy3 and ZakTOWNS have 98.LFL, but no other game
			if (g->id == GID_LOOM && fileMD5Map.contains("98.LFL"))
				return false;


		} else {
			// TODO: Unknown file header, deal with it. Maybe an unencrypted
			// variant...
			// Anyway, we don't know to deal with the file, so we
			// just skip it.
		}
	} else if (file == "000.LFL") {
		// Used in V4
		// Candidates: monkeyEGA, pass, monkeyVGA, loomcd

		if (g->version != 4)
			return false;

		/*
		For all of them, we have:
		_numGlobalObjects 1000
		_numRooms 99
		_numCostumes 199
		_numScripts 199
		_numSounds 199
		
		Any good ideas to distinguish those? Maybe by the presence / absence
		of some files?
		At least PASS and the monkeyEGA demo differ by 903.LFL missing...
		And the count of DISK??.LEC files differs depending on what version
		you have (4 or 8 floppy versions). 
		loomcd of course shipped on only one "disc".
		
		pass: 000.LFL, 901.LFL, 902.LFL, 904.LFL, disk01.lec
		monkeyEGA:  000.LFL, 901-904.LFL, DISK01-09.LEC
		monkeyEGA DEMO: 000.LFL, 901.LFL, 902.LFL, 904.LFL, disk01.lec
		monkeyVGA: 000.LFL, 901-904.LFL, DISK01-04.LEC
		loomcd: 000.LFL, 901-904.LFL, DISK01.LEC
		*/

		const bool has903LFL = fileMD5Map.contains("903.LFL");
		const bool hasDisk02 = fileMD5Map.contains("DISK02.LEC");
		
		// There is not much we can do based on the presence / absence
		// of files. Only that if 903.LFL is present, it can't be PASS;
		// and if DISK02.LEC is present, it can't be LoomCD
		if (g->id == GID_PASS              && !has903LFL && !hasDisk02) {
		} else if (g->id == GID_LOOM       &&  has903LFL && !hasDisk02) {
		} else if (g->id == GID_MONKEY_VGA) {
		} else if (g->id == GID_MONKEY_EGA) {
		} else
			return false;
	} else {
		// Must be a V5+ game
		if (g->version < 5)
			return false;

		// So at this point the gameid is determined, but not necessarily
		// the variant!
		
		// TODO: Add code that handles this, at least for the non-HE games.
		// Note sure how realistic it is to correctly detect HE-game
		// variants, would require me to look at a sufficiently large
		// sample collection of HE games (assuming I had the time :).
		
		// TODO: For Mac versions in container file, we can sometimes
		// distinguish the demo from the regular version by looking
		// at the content of the container file and then looking for
		// the *.000 file in there.
	}

	return true;
}


} // End of namespace Scumm

#pragma mark -
#pragma mark --- Plugin code ---
#pragma mark -


using namespace Scumm;

GameList Engine_SCUMM_gameIDList() {
	const PlainGameDescriptor *g = gameDescriptions;
	GameList games;
	while (g->gameid) {
		games.push_back(GameDescriptor(g->gameid, g->description));
		g++;
	}
	return games;
}

GameDescriptor Engine_SCUMM_findGameID(const char *gameid) {
	// First search the list of supported game IDs.
	const PlainGameDescriptor *g = gameDescriptions;
	while (g->gameid) {
		if (0 == scumm_stricmp(gameid, g->gameid))
			return GameDescriptor(g->gameid, g->description);
		g++;
	}

	// If we didn't find the gameid in the main list, check if it
	// is an obsolete game id.
	GameDescriptor gs;
	const ObsoleteGameID *o = obsoleteGameIDsTable;
	while (o->from) {
		if (0 == scumm_stricmp(gameid, o->from)) {
			gs["gameid"] = gameid;
			gs["description"] = "Obsolete game ID";
			return gs;
		}
		o++;
	}
	return gs;
}


GameList Engine_SCUMM_detectGames(const FSList &fslist) {
	GameList detectedGames;
	Common::List<DetectorResult> results;

	detectGames(fslist, results, 0);

	// TODO: We still don't handle the FM-TOWNS demos (like zakloom) very well.
	// In particular, they are detected as ZakTowns, which is bad.
	
	for (Common::List<DetectorResult>::iterator x = results.begin(); x != results.end(); ++x) {
		GameDescriptor dg(x->game.gameid, findDescriptionFromGameID(x->game.gameid),
				x->language, x->game.platform);
		dg.updateDesc(x->extra);	// Append additional information, if set, to the description.

		// Compute and set the preferred target name for this game.
		// Based on generateComplexID() in advancedDetector.cpp.
		Common::String res(x->game.gameid);

		if (x->game.preferredTag) {
			res = res + "-" + x->game.preferredTag;
		}

		if (x->game.features & GF_DEMO) {
			res = res + "-demo";
		}

		// Append the platform, if a non-standard one has been specified.
		if (x->game.platform != Common::kPlatformPC && x->game.platform != Common::kPlatformUnknown) {
			// HACK: For CoMI, it's pointless to encode the fact that it's for Windows
			if (x->game.id != GID_CMI)
				res = res + "-" + Common::getPlatformAbbrev(x->game.platform);
		}

		// Append the language, if a non-standard one has been specified
		if (x->language != Common::EN_ANY && x->language != Common::UNK_LANG) {
			res = res + "-" + Common::getLanguageCode(x->language);
		}

		dg["preferredtarget"] = res;

		detectedGames.push_back(dg);
	}

	return detectedGames;
}

/**
 * Create a ScummEngine instance, based on the given detector data.
 *
 * This is heavily based on our MD5 detection scheme.
 */
PluginError Engine_SCUMM_create(OSystem *syst, Engine **engine) {
	assert(syst);
	assert(engine);
	const char *gameid = ConfMan.get("gameid").c_str();

	// We start by checking whether the specified game ID is obsolete.
	// If that is the case, we automatically upgrade the target to use
	// the correct new game ID (and platform, if specified).
	for (const ObsoleteGameID *o = obsoleteGameIDsTable; o->from; ++o) {
		if (!scumm_stricmp(gameid, o->from)) {
			// Match found, perform upgrade
			gameid = o->to;
			ConfMan.set("gameid", o->to);

			if (o->platform != Common::kPlatformUnknown)
				ConfMan.set("platform", Common::getPlatformCode(o->platform));

			warning("Target upgraded from game ID %s to %s", o->from, o->to);
			ConfMan.flushToDisk();
			break;
		}
	}

	// Fetch the list of files in the current directory
	FSList fslist;
	FilesystemNode dir(ConfMan.get("path"));
	if (!dir.listDir(fslist, FilesystemNode::kListFilesOnly)) {
		return kInvalidPathError;
	}

	// Invoke the detector, but fixed to the specified gameid.
	Common::List<DetectorResult> results;
	detectGames(fslist, results, gameid);

	// Unable to locate game data
	if (results.empty()) {
		return kNoGameDataFoundError;
	}

	// No unique match found. If a platform override is present, try to
	// narrow down the list a bit more.
	if (results.size() > 1 && ConfMan.hasKey("platform")) {
		Common::Platform platform = Common::parsePlatform(ConfMan.get("platform"));
		for (Common::List<DetectorResult>::iterator x = results.begin(); x != results.end(); ) {
			if (x->game.platform != platform) {
				x = results.erase(x); 
			} else {
				++x;
			}
		}
	}

	// If we narrowed it down too much, abort
	if (results.empty()) {
		warning("Engine_SCUMM_create: Game data inconsistent with platform override");
		return kNoGameDataFoundError;
	}

	// Still no unique match found -> print a warning
	if (results.size() > 1) {
		warning("Engine_SCUMM_create: No unique game candidate found, using first one");
	}

	// Simply use the first match
	DetectorResult res(*(results.begin()));
	debug(1, "Using gameid %s, variant %s, extra %s", res.game.gameid, res.game.variant, res.extra);

	// Print the MD5 of the game; either verbose using printf, in case of an
	// unknown MD5, or with a medium debug level in case of a known MD5 (for
	// debugging purposes).
	if (!findInMD5Table(res.md5.c_str())) {
		printf("Your game version appears to be unknown. Please, report the following\n");
		printf("data to the ScummVM team along with name of the game you tried to add\n");
		printf("and its version/language/etc.:\n");
		
		printf("  SCUMM gameid '%s', file '%s', MD5 '%s'\n\n",
				res.game.gameid,
				generateFilenameForDetection(res.fp.pattern, res.fp.genMethod).c_str(),
				res.md5.c_str());
	} else {
		debug(1, "Using MD5 '%s'", res.md5.c_str());
	}

	// Check for a user override of the platform. We allow the user to override
	// the platform, to make it possible to add games which are not yet in 
	// our MD5 database but require a specific platform setting.
	// TODO: Do we really still need / want the platform override ?
	if (ConfMan.hasKey("platform"))
		res.game.platform = Common::parsePlatform(ConfMan.get("platform"));

	// Language override
	if (ConfMan.hasKey("language"))
		res.language = Common::parseLanguage(ConfMan.get("language"));

	// V3 FM-TOWNS games *always* should use the corresponding music driver,
	// anything else makes no sense for them.
	// TODO: Maybe allow the null driver, too?
	if (res.game.platform == Common::kPlatformFMTowns && res.game.version == 3)
		res.game.midi = MDT_TOWNS;

	// Finally, we have massaged the GameDescriptor to our satisfaction, and can
	// instantiate the appropriate game engine. Hooray!
	switch (res.game.version) {
	case 0:
		*engine = new ScummEngine_v0(syst, res);
		break;
	case 1:
	case 2:
		*engine = new ScummEngine_v2(syst, res);
		break;
	case 3:
		if ((res.game.features & GF_OLD256) || res.game.platform == Common::kPlatformPCEngine) 
			*engine = new ScummEngine_v3(syst, res);
		else
			*engine = new ScummEngine_v3old(syst, res);
		break;
	case 4:
		*engine = new ScummEngine_v4(syst, res);
		break;
	case 5:
		*engine = new ScummEngine_v5(syst, res);
		break;
	case 6:
		switch (res.game.heversion) {
#ifndef DISABLE_HE
		case 200:
			*engine = new ScummEngine_vCUPhe(syst, res);
			break;
		case 100:
			*engine = new ScummEngine_v100he(syst, res);
			break;
		case 99:
			*engine = new ScummEngine_v99he(syst, res);
			break;
		case 98:
		case 95:
		case 90:
			*engine = new ScummEngine_v90he(syst, res);
			break;
		case 85:
		case 80:
			*engine = new ScummEngine_v80he(syst, res);
			break;
		case 73:
		case 72:
			*engine = new ScummEngine_v72he(syst, res);
			break;
		case 71:
			*engine = new ScummEngine_v71he(syst, res);
			break;
		case 70:
			*engine = new ScummEngine_v70he(syst, res);
			break;
#endif
#ifndef PALMOS_68K
		case 61:
			*engine = new ScummEngine_v60he(syst, res);
			break;
#endif
		default:
			*engine = new ScummEngine_v6(syst, res);
		}
		break;
#ifndef DISABLE_SCUMM_7_8
	case 7:
		*engine = new ScummEngine_v7(syst, res);
		break;
	case 8:
		*engine = new ScummEngine_v8(syst, res);
		break;
#endif
	default:
		error("Engine_SCUMM_create(): Unknown version of game engine");
	}

	return kNoError;
}

REGISTER_PLUGIN(SCUMM, "Scumm Engine",
				"LucasArts SCUMM Games (C) LucasArts\n"
				"Humongous SCUMM Games (C) Humongous" );

#ifdef PALMOS_68K
#include "scumm_globals.h"

_GINIT(Scumm_md5table)
_GSETPTR(md5table, GBVARS_MD5TABLE_INDEX, MD5Table, GBVARS_SCUMM)
_GEND

_GRELEASE(Scumm_md5table)
_GRELEASEPTR(GBVARS_MD5TABLE_INDEX, GBVARS_SCUMM)
_GEND

#endif
