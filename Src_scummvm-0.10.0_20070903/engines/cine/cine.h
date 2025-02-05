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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/cine/cine.h $
 * $Id: cine.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef CINE_H
#define CINE_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/file.h"
#include "common/util.h"

#include "engines/engine.h"

#include "cine/texte.h"
#include "cine/rel.h"
#include "cine/script.h"
#include "cine/part.h"
#include "cine/prc.h"
#include "cine/msg.h"
#include "cine/bg.h"
#include "cine/pal.h"
#include "cine/gfx.h"
#include "cine/anim.h"

//#define DUMP_SCRIPTS

namespace Cine {

enum CineGameType {
	GType_FW = 1,
	GType_OS
};

enum CineGameFeatures {
	GF_CD =   1 << 0,
	GF_DEMO = 1 << 1,
	GF_ALT_FONT = 1 << 2
};

struct CINEGameDescription;

class CineEngine : public Engine {

protected:
	int init();
	int go();
	void shutdown();
	
	bool initGame();

public:
	CineEngine(OSystem *syst);
	virtual ~CineEngine();

	int getGameType() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;

	bool loadSaveDirectory(void);
	void makeSystemMenu(void);

	const CINEGameDescription *_gameDescription;
	Common::File _partFileHandle;

private:
	void initialize(void);
	bool makeLoad(char *saveName);
	void mainLoop(int bootScriptIdx);

	bool _preLoad;
};

extern CineEngine *g_cine;

#define BOOT_PRC_NAME "AUTO00.PRC"

enum {
	VAR_MOUSE_X_MODE = 253,
	VAR_MOUSE_X_POS = 249,
	VAR_MOUSE_Y_MODE = 251,
	VAR_MOUSE_Y_POS = 250
};

enum {
	MOUSE_CURSOR_NORMAL = 0,
	MOUSE_CURSOR_DISK,
	MOUSE_CURSOR_CROSS
};

enum {
	kCineDebugScript = 1 << 0
};

enum {
	kCmpEQ = (1 << 0),
	kCmpGT = (1 << 1),
	kCmpLT = (1 << 2)
};


extern Common::SaveFileManager *g_saveFileMan; // TEMP

} // End of namespace Cine

#endif
