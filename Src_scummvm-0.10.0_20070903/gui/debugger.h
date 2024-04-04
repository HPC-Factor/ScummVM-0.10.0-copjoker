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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/gui/debugger.h $
 * $Id: debugger.h 27024 2007-05-30 21:56:52Z fingolfin $
 */

#ifndef GUI_DEBUGGER_H
#define GUI_DEBUGGER_H

namespace GUI {

// Choose between text console or ScummConsole
#define USE_CONSOLE	1

#ifdef USE_CONSOLE
class ConsoleDialog;
#endif

class Debugger {
public:
	Debugger();
	virtual ~Debugger();

	int DebugPrintf(const char *format, ...);

	virtual void onFrame();

	virtual void attach(const char *entry = 0);
	bool isAttached() const { return _isAttached; }

protected:
	class Debuglet {
	public:
		virtual ~Debuglet() {}
		virtual bool operator()(int argc, const char **argv) = 0;
	};
	
	template <class T>
	class DelegateDebuglet : public Debuglet {
		typedef bool (T::*Method)(int argc, const char **argv);

		T *_delegate;
		const Method _method;
	public:
		DelegateDebuglet(T *delegate, Method method)
			: _delegate(delegate), _method(method) {
			assert(delegate != 0);
		}
		virtual bool operator()(int argc, const char **argv) {
			return (_delegate->*_method)(argc, argv);
		};
	};
	
	// Convenicence macro for registering a method of a debugger class
	// as the current command.
	#define WRAP_METHOD(cls, method) \
		new DelegateDebuglet<cls>(this, &cls::method)

	enum {
		DVAR_BYTE,
		DVAR_INT,
		DVAR_BOOL,
		DVAR_INTARRAY,
		DVAR_STRING
	};

	struct DVar {
		char name[30];
		void *variable;
		int type, optional;
	};

	struct DCmd {
		char name[30];
		Debuglet *debuglet;
	};

	int _frame_countdown;
	bool _detach_now;
	
private:
	// TODO: Consider replacing the following two arrays by a Hashmap
	int _dvar_count;
	DVar _dvars[256];

	int _dcmd_count;
	DCmd _dcmds[256];

	bool _isAttached;
	char *_errStr;
	bool _firstTime;
	GUI::ConsoleDialog *_debuggerDialog;

protected:
	// Hook for subclasses: Called just before enter() is run
	virtual void preEnter() {}

	// Hook for subclasses: Called just after enter() was run
	virtual void postEnter() {}

	// Hook for subclasses: Process the given command line.
	// Should return true if and only if argv[0] is a known command and was
	// handled, false otherwise.
	virtual bool handleCommand(int argc, const char **argv, bool &keepRunning);

	
private:
	void detach();
	void enter();

	bool parseCommand(const char *input);
	bool tabComplete(const char *input, char*& completion);

protected:
	void DVar_Register(const char *varname, void *pointer, int type, int optional);
	void DCmd_Register(const char *cmdname, Debuglet *debuglet);

	bool Cmd_Exit(int argc, const char **argv);
	bool Cmd_Help(int argc, const char **argv);
	bool Cmd_DebugFlagsList(int argc, const char **argv);
	bool Cmd_DebugFlagEnable(int argc, const char **argv);
	bool Cmd_DebugFlagDisable(int argc, const char **argv);

#if USE_CONSOLE
private:
	static bool debuggerInputCallback(GUI::ConsoleDialog *console, const char *input, void *refCon);
	static bool debuggerCompletionCallback(GUI::ConsoleDialog *console, const char *input, char*& completion, void *refCon);
#endif
};

}	// End of namespace GUI

#endif
