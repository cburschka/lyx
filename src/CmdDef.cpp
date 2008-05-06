/**
 * \file CmdDef.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bernhard Roider
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "CmdDef.h"

#include "LyXAction.h"
#include "Lexer.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/lstrings.h"

#include <string>

using namespace std;
using namespace lyx::support;

namespace lyx {

bool CmdDef::read(string const & def_file)
{
	enum {
		BN_DEFFILE,
		BN_DEFINE
	};

	LexerKeyword cmdDefTags[] = {
		{ "\\def_file", BN_DEFFILE },
		{ "\\define", BN_DEFINE }
	};

	Lexer lex(cmdDefTags);
	FileName const tmp(i18nLibFileSearch("commands", def_file, "def"));
	lex.setContext("CmdDef::read");
	lex.setFile(tmp);
	if (!lex.isOK()) {
		LYXERR0( "CmdDef::read: cannot open def file:" << tmp);
		return false;
	}

	bool error = false;
	while (lex.isOK()) {
		switch (lex.lex()) {
		case Lexer::LEX_UNDEF:
			lex.printError("Unknown tag");
			error = true;
			continue;
		case Lexer::LEX_FEOF:
			continue;
		case BN_DEFINE:
		{
			string name, def;

			if (lex.next()) {
				name = lex.getString();
			} else {
				lex.printError("BN_DEFINE: Missing command name");
				error = true;
				break;
			}

			if (lex.next(true)) {
				def = lex.getString();
			} else {
				lex.printError("BN_DEFINE: missing command definition");
				error = true;
				break;
			}

			newCmdDefResult e = newCmdDef(name, def);
			switch (e) {
			case CmdDefNameEmpty:
				lex.printError("BN_DEFINE: Command name is empty");
				error = true;
				break;
			case CmdDefExists:
				lex.printError("BN_DEFINE: Command `" + name + "' already defined");
				error = true;
				break;
			case CmdDefInvalid:
				lex.printError("BN_DEFINE: Command definition for `" + name + "' is not valid");
				error = true;
				break;
			case CmdDefOk:
				break;
			}

			break;
		}
		case BN_DEFFILE:
			if (lex.next()) {
				string const tmp = lex.getString();
				error |= !read(tmp);
			} else {
				lex.printError("BN_DEFFILE: Missing file name");
				error = true;
				break;

			}
			break;
		}
	}

	if (error)
		LYXERR0("CmdDef::read: error while reading def file:" << tmp);
	return !error;
}


bool CmdDef::lock(string const & name, FuncRequest & func)
{
	if (cmdDefMap.empty()) {
		func = FuncRequest::unknown;
		return false;
	}

	string const name2 = trim(name);

	if (lockSet.find(name2) != lockSet.end()) {
		func = FuncRequest::noaction;
		return false;
	}

	CmdDefMap::const_iterator pos = cmdDefMap.find(name2);

	if (pos == cmdDefMap.end()) {
		func = FuncRequest::unknown;
		return false;
	}

	lockSet.insert(name2);
	func = pos->second;
	return true;
}


void CmdDef::release(string const & name)
{
	string const name2 = trim(name);
	lockSet.erase(name2);
}


CmdDef::newCmdDefResult CmdDef::newCmdDef(string const & name, 
										  string const & def)
{
	string const name2 = trim(name);

	if (name2.empty()) 
		return CmdDefNameEmpty;

	if (cmdDefMap.find(name) != cmdDefMap.end())
		return CmdDefExists;

	FuncRequest	func = lyxaction.lookupFunc(def);
	if (func.action == LFUN_NOACTION
		|| func.action == LFUN_UNKNOWN_ACTION) {
			return CmdDefInvalid;
	}

	cmdDefMap[name2] = func;

	return CmdDefOk;
}


} // namespace lyx
