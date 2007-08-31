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

#include "debug.h"
#include "Lexer.h"
#include "ModuleList.h"

#include "support/docstring.h"
#include "support/filetools.h"
#include "support/lstrings.h"

#include <algorithm>
	
using std::endl;
using std::map;
using std::string;
using std::vector;

namespace lyx{

using support::FileName;
using support::libFileSearch;
using support::makeDisplayPath;

//global variable: module list
ModuleList moduleList;


// used when sorting the module list.
class moduleSorter
	: public std::binary_function<LyXModule, LyXModule, int>
{
	public:
		int operator()(LyXModule const & lm1,
								 LyXModule const & lm2) const
		{
			return (lm1.name < lm2.name);
		}
};


//Much of this is borrowed from TextClassList::read()
bool ModuleList::load() {
	support::FileName const real_file = libFileSearch("", "lyxmodules.lst");
	LYXERR(Debug::TCLASS) << "Reading modules from `"
			<< real_file << '\'' << endl;

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
	LYXERR(Debug::TCLASS) << "Starting parsing of lyxmodules.lst" << endl;
	while (lex.isOK() && !finished) {
		LYXERR(Debug::TCLASS) << "\tline by line" << endl;
		switch (lex.lex()) {
		case Lexer::LEX_FEOF:
			finished = true;
			break;
		default:
			string const modName = lex.getString();
			LYXERR(Debug::TCLASS) << "Module name: " << modName << endl;
			if (lex.next()) {
				string const fname = lex.getString();
				LYXERR(Debug::TCLASS) << "Filename: " << fname << endl;
				if (lex.next()) {
					string const desc = lex.getString();
					LYXERR(Debug::TCLASS) << "Description: " << desc << endl;
					//FIXME Add packages
					if (lex.next()) {
						string packages = lex.getString();
						LYXERR(Debug::TCLASS) << "Packages: " << packages << endl;
						vector<string> pkgs;
						while (!packages.empty()) {
							string p;
							packages = support::split(packages, p, ',');
							pkgs.push_back(p);
						}
						// This code is run when we have
						// modName, fname, desc, and pkgs
						addLayoutModule(modName, fname, desc, pkgs);
					}
				}
			}
		} // end switch
	} //end while
	
	LYXERR(Debug::TCLASS) << "End of parsing of lyxmodules.lst" << endl;

	if (!moduleList.empty())
		std::sort(moduleList.begin(), moduleList.end(), moduleSorter());
	return true;
}


void ModuleList::addLayoutModule(string moduleName, 
		string filename, string description, vector<string> pkgs) {
	LyXModule lm;
	lm.name = moduleName;
	lm.filename = filename;
	lm.description = description;
	lm.packageList = pkgs;
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


LyXModule * ModuleList::operator[](string const str) {
	LyXModuleList::iterator it = modlist_.begin();
	for (; it != modlist_.end(); ++it)
		if (it->name == str) {
			LyXModule & mod = *it;
			return &mod;
		}
	return 0;
}

}
