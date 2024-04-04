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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/gui/eval.h $
 * $Id: eval.h 27024 2007-05-30 21:56:52Z fingolfin $
 *
 */

#ifndef GUI_EVAL_H
#define GUI_EVAL_H

#include "common/stdafx.h"
#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace GUI {

using Common::String;
using Common::HashMap;

enum {
	EVAL_UNDEF_VAR  = -13375,
	EVAL_STRING_VAR = -13376
};

class Eval {
public:
	Eval();
	~Eval();

	int eval(const String &input, const String &section, const String &name, int startpos);
	void setVar(const String &section, const String &name, const String &value);

	void setParent(const String &name);

	void setVar(const String &name, int val) { _vars[name] = val; }
	void setStringVar(const String &name, const String &val) { _strings[name] = val; }
	void setAlias(const Common::String &name, const String &val) { _aliases[name] = val; }

	int getVar(const Common::String &s) { return getVar_(s); }
	int getVar(const Common::String &s, int def) {
		int val = getVar_(s);
		return (val == EVAL_UNDEF_VAR) ? def : val;
	}

	const String &getStringVar(const Common::String &name) { return _strings[name]; }

	uint getNumVars() { return _vars.size(); }

	void reset();

	char *lastToken() { return _token; }

	typedef HashMap<String, int, Common::CaseSensitiveString_Hash, Common::CaseSensitiveString_EqualTo> VariablesMap;
	typedef HashMap<String, String, Common::CaseSensitiveString_Hash, Common::CaseSensitiveString_EqualTo> AliasesMap;
	typedef HashMap<String, String, Common::CaseSensitiveString_Hash, Common::CaseSensitiveString_EqualTo> StringsMap;

private:
	enum TokenTypes {
		tNone,
		tDelimiter,
		tVariable,
		tNumber,
		tString
	};

	enum EvalErrors {
		eSyntaxError,
		eExtraBracket,
		eUnclosedBracket,
		eBadExpr,
		eUndefVar,
		eMissingQuote
	};


	void getToken();
	void level2(int *);
	void level3(int *);
	void level4(int *);
	void level5(int *);
	void primitive(int *);
	void arith(char op, int *r, int *h);
	void unary(char op, int *r);
	void exprError(EvalErrors error);
	int getVar_(const Common::String &s, bool includeAliases = true);
	int getBuiltinVar(const char *s);
	void loadConstants();

	char _input[256];
	String _section;
	String _name;

	int _startpos;

	TokenTypes _tokenType;
	int _pos;

	char _token[256];

	AliasesMap _aliases;
	VariablesMap _vars;
	StringsMap _strings;
};

} // end of namespace GUI

#endif
