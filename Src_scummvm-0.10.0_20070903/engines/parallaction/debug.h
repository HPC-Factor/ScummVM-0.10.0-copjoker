
#ifndef PARALLACTION_DEBUGGER_H
#define PARALLACTION_DEBUGGER_H

#include "gui/debugger.h"

namespace Parallaction {

class Parallaction;

class Debugger : public GUI::Debugger {
public:
	Debugger(Parallaction *vm);
	virtual ~Debugger() {}  // we need this for __SYMBIAN32__ archaic gcc/UIQ

protected:
	Parallaction *_vm;

	virtual void preEnter();
	virtual void postEnter();

	bool Cmd_DebugLevel(int argc, const char **argv);
	bool Cmd_Location(int argc, const char **argv);
	bool Cmd_Give(int argc, const char **argv);
	bool Cmd_Jobs(int argc, const char **argv);
	bool Cmd_Zones(int argc, const char **argv);
	bool Cmd_Animations(int argc, const char **argv);

};

} // End of namespace Parallaction

#endif
