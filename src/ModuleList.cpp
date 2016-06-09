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
#include "support/gettext.h"
#include "support/filetools.h"
#include "support/lstrings.h"

#include <algorithm>

using namespace std;
using namespace lyx::support;

namespace lyx {


//global variable: module list
ModuleList theModuleList;


LyXModule::LyXModule(string const & n, string const & i,
	                   string const & d, vector<string> const & p,
	                   vector<string> const & r, vector<string> const & e,
	                   string const & c):
	name_(n), id_(i), description_(d), package_list_(p),
	required_modules_(r), excluded_modules_(e), category_(c),
	checked_(false), available_(false)
{
	filename_ = id_ + ".module";
}


vector<string> LyXModule::prerequisites() const
{
	if (!checked_)
		isAvailable();
	return prerequisites_;
}


bool LyXModule::isAvailable() const
{
	if (package_list_.empty())
		return true;
	if (checked_)
		return available_;
	checked_ = true;
	available_ = true;
	//check whether all of the required packages are available
	vector<string>::const_iterator it  = package_list_.begin();
	vector<string>::const_iterator end = package_list_.end();
	for (; it != end; ++it) {
		if (!LaTeXFeatures::isAvailable(*it)) {
			available_ = false;
			prerequisites_.push_back(*it);
		}
	}
	return available_;
}


bool LyXModule::isCompatible(string const & modname) const
{
	// do we exclude it?
	if (find(excluded_modules_.begin(), excluded_modules_.end(), modname) !=
			excluded_modules_.end())
		return false;

	LyXModule const * const lm = theModuleList[modname];
	if (!lm)
		return true;

	// does it exclude us?
	vector<string> const excmods = lm->getExcludedModules();
	if (find(excmods.begin(), excmods.end(), id_) != excmods.end())
		return false;

	return true;
}


bool LyXModule::areCompatible(string const & mod1, string const & mod2)
{
	LyXModule const * const lm1 = theModuleList[mod1];
	if (lm1)
		return lm1->isCompatible(mod2);
	LyXModule const * const lm2 = theModuleList[mod2];
	if (lm2)
		return lm2->isCompatible(mod1);
	// Can't check it either way.
	return true;
}


// used when sorting the module list.
class ModuleSorter {
public:
	int operator()(LyXModule const & lm1, LyXModule const & lm2) const
	{
		return _(lm1.getName()) < _(lm2.getName());
	}
};


// Much of this is borrowed from LayoutFileList::read()
bool ModuleList::read()
{
	FileName const real_file = libFileSearch("", "lyxmodules.lst");
	LYXERR(Debug::TCLASS, "Reading modules from `" << real_file << '\'');

	if (real_file.empty()) {
		LYXERR0("unable to find modules file `lyxmodules.lst'.\n"
			<< "No modules will be available.");
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
			<< to_utf8(makeDisplayPath(real_file.absFileName(), 1000))
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
			string const modname = lex.getString();
			LYXERR(Debug::TCLASS, "Module name: " << modname);
			if (!lex.next())
				break;
			string const fname = lex.getString();
			LYXERR(Debug::TCLASS, "Filename: " << fname);
			if (!lex.next(true))
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
			if (!lex.next())
				break;
			string const catgy = lex.getString();
			LYXERR(Debug::TCLASS, "Category: " << catgy);
			// This code is run when we have
			// modName, fname, desc, pkgs, req, exc, and catgy
			addLayoutModule(modname, fname, desc, pkgs, req, exc, catgy);
		} // end switch
	} //end while

	LYXERR(Debug::TCLASS, "End of parsing of lyxmodules.lst");

	if (!theModuleList.empty())
		sort(theModuleList.begin(), theModuleList.end(), ModuleSorter());
	return true;
}


void ModuleList::addLayoutModule(string const & modname,
	string const & filename, string const & description,
	vector<string> const & pkgs, vector<string> const & req,
	vector<string> const & exc, string const & catgy)
{
	LyXModule lm(modname, filename, description, pkgs, req, exc, catgy);
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


LyXModule const * ModuleList::operator[](string const & str) const
{
	LyXModuleList::const_iterator it = modlist_.begin();
	for (; it != modlist_.end(); ++it)
		if (it->getID() == str) {
			LyXModule const & mod = *it;
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
