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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/engines/engine.h $
 * $Id: engine.h 27024 2007-05-30 21:56:52Z fingolfin $
 */

#ifndef BASE_ENGINE_H
#define BASE_ENGINE_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/str.h"

class OSystem;
namespace Audio {
	class Mixer;
}
namespace Common {
	class EventManager;
	class SaveFileManager;
	class TimerManager;
}
namespace GUI {
	class Debugger;
}

class Engine {
public:
	OSystem *_system;
	Audio::Mixer *_mixer;
	Common::TimerManager * _timer;

protected:
	Common::EventManager *_eventMan;
	Common::SaveFileManager *_saveFileMan;

	const Common::String _targetName; // target name for saves
	const Common::String _gameDataPath;

private:
	int _autosavePeriod;

public:
	Engine(OSystem *syst);
	virtual ~Engine();

	/**
	 * Init the engine.
	 * @return 0 for success, else an error code.
	 */
	virtual int init() = 0;

	/**
	 * Start the main engine loop.
	 * The return value is not yet used, but could indicate whether the user
	 * wants to return to the launch or to fully quit ScummVM.
	 * @return a result code
	 */
	virtual int go() = 0;

	/** Specific for each engine: prepare error string. */
	virtual void errorString(const char *buf_input, char *buf_output);

	void initCommonGFX(bool defaultTo1XScaler);

	/** On some systems, check if the game appears to be run from CD. */
	void checkCD();

	/* Indicate if an autosave should be performed. */
	bool shouldPerformAutoSave(int lastSaveTime);

	/** Initialized graphics and shows error message. */
	void GUIErrorMessage(const Common::String msg);

	/**
	 * Return the engine's debugger instance, if any. Used by error() to
	 * invoke the debugger when a severe error is reported.
	 */
	virtual GUI::Debugger *getDebugger() { return 0; }
};

extern Engine *g_engine;

#endif
