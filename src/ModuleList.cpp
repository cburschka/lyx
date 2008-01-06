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

#include "support/debug.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"

#include "support/FileName.h"
#include "support/filetools.h"
#include "support/lstrings.h"

#include <algorithm>
#include <ostream>
	
using namespace std;
using namespace lyx::support;

namespace lyx {


//global variable: module list
ModuleList moduleList;


LyXModule::LyXModule(string n, string f, string d,
	          vector<string> p) : 
	name(n), filename(f), description(d), packageList(p), checked(false)
{}


bool LyXModule::isAvailable() {
	if (packageList.empty())
		return true;
	if (checked)
		return available;
	checked = true;
	vector<string>::const_iterator it  = packageList.begin();
	vector<string>::const_iterator end = packageList.end(); 
	for (; it != end; ++it) {
		if (!LaTeXFeatures::isAvailable(*it))
			available = false;
			return available;
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
		return lm1.name < lm2.name;
	}
};


//Much of this is borrowed from TextClassList::read()
bool ModuleList::load()
{
	FileName const real_file = libFileSearch("", "lyxmodules.lst");
	LYXERR(Debug::TCLASS, "Reading modules from `" << real_file << '\'');

	if (real_file.empty()) {
		lyxerr << "ModuleList::load(): unable to find "
				"modules file  `"
				<< to_utf8(makeDisplayPath(real_file.absFilename(), 1000))
				<< "'.\nNo modules will be available." << endl;
		return false;
	}

	Lexer lex(0, 0);
	if (!lex.setFile(real_file)) {
		lyxerr << "ModuleList::load():"
				"lyxlex was not able to set file: "
				<< real_file << ".\nNo modules will be available." << endl;
		return false;
	}

	if (!lex.isOK()) {
		lyxerr << "ModuleList::load():" <<
				"unable to open modules file  `"
				<< to_utf8(makeDisplayPath(real_file.absFilename(), 1000))
				<< "'\nNo modules will be available."
				<< endl;
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
			string packages = lex.getString();
			LYXERR(Debug::TCLASS, "Packages: " << packages);
			vector<string> pkgs;
			while (!packages.empty()) {
				string p;
				packages = split(packages, p, ',');
				pkgs.push_back(p);
			}
			// This code is run when we have
			// modName, fname, desc, and pkgs
			addLayoutModule(modName, fname, desc, pkgs);
		} // end switch
	} //end while
	
	LYXERR(Debug::TCLASS, "End of parsing of lyxmodules.lst");

	if (!moduleList.empty())
		sort(moduleList.begin(), moduleList.end(), ModuleSorter());
	return true;
}


void ModuleList::addLayoutModule(string const & moduleName, 
	string const & filename, string const & description,
	vector<string> const & pkgs)
{
	LyXModule lm(moduleName, filename, description, pkgs);
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


LyXModule * ModuleList::operator[](string const & str)
{
	LyXModuleList::iterator it = modlist_.begin();
	for (; it != modlist_.end(); ++it)
		if (it->name == str) {
			LyXModule & mod = *it;
			return &mod;
		}
	return 0;
}

} // namespace lyx
