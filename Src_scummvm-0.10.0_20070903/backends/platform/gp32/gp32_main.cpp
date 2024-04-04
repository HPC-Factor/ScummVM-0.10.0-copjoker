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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/platform/gp32/gp32_main.cpp $
 * $Id: gp32_main.cpp 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#include "stdafx.h"
#include "common/scummsys.h"

#include "common/config-manager.h"

#include "base/main.h"

#include "gp32std.h"
#include "gp32std_grap.h"

#include "gp32_launcher.h"
#include "gp32_osys.h"
#include "globals.h"

GlobalVars g_vars;

void init() {
	gp_setCpuSpeed(40); // Default CPU Speed

	GpGraphicModeSet(16, NULL);

	//if (type == NLU || type == FLU || type == BLU)
	//	gp_initFramebuffer(frameBuffer, 16, 85);
	//else if (type == BLUPLUS)
	//	gp_initFramebufferBP(frameBuffer, 16, 85);
	// else
	//	error("Invalid Console");
	gp_initFrameBuffer();

	GpFatInit();
	GpRelativePathSet("gp:\\gpmm");
}

void GpMain(void *arg) {
	extern void memBlockInit();
	memBlockInit();

	init();

	readConfigVars();

	splashScreen();

	//doConfig
	gp_initGammaTable((float)g_vars.gammaRamp / 10000);
	gp_setCpuSpeed(g_vars.cpuSpeed);
	GPDEBUG("Set CPU Speed: %d", g_vars.cpuSpeed);

	// FOR DEBUG PURPOSE!
	//int argc = 4;
	//char *argv[] = { "scummvm", "-enull", "-pgp:\\game\\dott\\", "tentacle" };
	//int argc = 2;
	//char *argv[] = { "scummvm", "-d5" };

	extern OSystem *OSystem_GP32_create();
	g_system = OSystem_GP32_create();
	assert(g_system);

	// Invoke the actual ScummVM main entry point:
	//int res = scummvm_main(argc, argv);
	int res = scummvm_main(1, NULL);

	extern void memBlockDeinit();
	memBlockDeinit();

	g_system->quit();	// TODO: Consider removing / replacing this!
	
	//return res;
}
