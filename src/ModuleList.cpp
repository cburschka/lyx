// -*- C++ -*-
/**
 * \file ModuleList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ModuleList.h"

#include "LaTeXFeatures.h"
#include "Lexer.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/lstrings.h"

#include <algorithm>
	
using namespace std;
using namespace lyx::support;

namespace lyx {


//global variable: module list
ModuleList moduleList;


LyXModule::LyXModule(string const & n, string const & i, 
	                   string const & d, vector<string> const & p,
	                   vector<string> const & r, vector<string> const & e):
	name(n), id(i), description(d), 
	packageList(p), requiredModules(r), excludedModules(e),
	checked(false)
{
	filename = id + ".module";
}


bool LyXModule::isAvailable() {
	if (packageList.empty())
		return true;
	if (checked)
		return available;
	checked = true;
	//check whether all of the required packages are available
	vector<string>::const_iterator it  = packageList.begin();
	vector<string>::const_iterator end = packageList.end(); 
	for (; it != end; ++it) {
		if (!LaTeXFeatures::isAvailable(*it)) {
			available = false;
			return available;
		}
	}
	available = true;
	return available;
}


// used when sorting the module list.
class ModuleSorter
{
public:
	int operator()(LyXModule const & lm1, LyXModule const & lm2) const
	{
		return lm1.getName() < lm2.getName();
	}
};


//Much of this is borrowed from LayoutFileList::read()
bool ModuleList::load()
{
	FileName const real_file = libFileSearch("", "lyxmodules.lst");
	LYXERR(Debug::TCLASS, "Reading modules from `" << real_file << '\'');

	if (real_file.empty()) {
		LYXERR0("unable to find modules file  `"
			<< to_utf8(makeDisplayPath(real_file.absFilename(), 1000))
			<< "'.\nNo modules will be available.");
		return false;
	}

	Lexer lex;
	if (!lex.setFile(real_file)) {
		LYXERR0("lyxlex was not able to set file: "
			<< real_file << ".\nNo modules will be available.");
		return false;
	}

	if (!lex.isOK()) {
		LYXERR0("unable to open modules file  `"
			<< to_utf8(makeDisplayPath(real_file.absFilename(), 1000))
			<< "'\nNo modules will be available.");
		return false;
	}

	bool finished = false;
	// Parse modules files
	LYXERR(Debug::TCLASS, "Starting parsing of lyxmodules.lst");
	while (lex.isOK() && !finished) {
		LYXERR(Debug::TCLASS, "\tline by line");
		switch (lex.lex()) {
		case Lexer::LEX_FEOF:
			finished = true;
			break;
		default:
			string const modName = lex.getString();
			LYXERR(Debug::TCLASS, "Module name: " << modName);
			if (!lex.next())
				break;
			string const fname = lex.getString();
			LYXERR(Debug::TCLASS, "Filename: " << fname);
			if (!lex.next())
				break;
			string const desc = lex.getString();
			LYXERR(Debug::TCLASS, "Description: " << desc);
			//FIXME Add packages
			if (!lex.next())
				break;
			string str = lex.getString();
			LYXERR(Debug::TCLASS, "Packages: " << str);
			vector<string> pkgs;
			while (!str.empty()) {
				string p;
				str = split(str, p, ',');
				pkgs.push_back(p);
			}
			if (!lex.next())
				break;
			str = lex.getString();
			LYXERR(Debug::TCLASS, "Required: " << str);
			vector<string> req;
			while (!str.empty()) {
				string p;
				str = split(str, p, '|');
				req.push_back(p);
			}
			if (!lex.next())
				break;
			str = lex.getString();
			LYXERR(Debug::TCLASS, "Excluded: " << str);
			vector<string> exc;
			while (!str.empty()) {
				string p;
				str = split(str, p, '|');
				exc.push_back(p);
			}
			// This code is run when we have
			// modName, fname, desc, pkgs, req, and exc
			addLayoutModule(modName, fname, desc, pkgs, req, exc);
		} // end switch
	} //end while
	
	LYXERR(Debug::TCLASS, "End of parsing of lyxmodules.lst");

	if (!moduleList.empty())
		sort(moduleList.begin(), moduleList.end(), ModuleSorter());
	return true;
}


void ModuleList::addLayoutModule(string const & moduleName, 
	string const & filename, string const & description,
	vector<string> const & pkgs, vector<string> const & req,
	vector<string> const & exc)
{
	LyXModule lm(moduleName, filename, description, pkgs, req, exc);
	modlist_.push_back(lm);
}


LyXModuleList::const_iterator ModuleList::begin() const
{
	return modlist_.begin();
}


LyXModuleList::iterator ModuleList::begin()
{
	return modlist_.begin();
}


LyXModuleList::const_iterator ModuleList::end() const
{
	return modlist_.end();
}


LyXModuleList::iterator ModuleList::end()
{
	return modlist_.end();
}


LyXModule * ModuleList::getModuleByName(string const & str)
{
	LyXModuleList::iterator it = modlist_.begin();
	for (; it != modlist_.end(); ++it)
		if (it->getName() == str) {
			LyXModule & mod = *it;
			return &mod;
		}
	return 0;
}

LyXModule * ModuleList::operator[](string const & str)
{
	LyXModuleList::iterator it = modlist_.begin();
	for (; it != modlist_.end(); ++it)
		if (it->getID() == str) {
			LyXModule & mod = *it;
			return &mod;
		}
	return 0;
}

} // namespace lyx
