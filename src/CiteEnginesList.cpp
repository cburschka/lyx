// -*- C++ -*-
/**
 * \file CiteEnginesList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Richard Heck
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "CiteEnginesList.h"

#include "Citation.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/gettext.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/Translator.h"

#include <algorithm>

using namespace std;
using namespace lyx::support;

namespace lyx {


//global variable: cite engine list
CiteEnginesList theCiteEnginesList;


LyXCiteEngine::LyXCiteEngine(string const & n, string const & i,
			     vector<string> const & cet, vector<string> const & dbs,
			     string const & d, vector<string> const & p,
			     vector<string> const & r, vector<string> const & e):
	name_(n), id_(i), engine_types_(cet), default_biblios_(dbs), description_(d),
	package_list_(p), required_engines_(r), excluded_engines_(e),
	checked_(false), available_(false)
{
	filename_ = id_ + ".citeengine";
}


vector<string> LyXCiteEngine::prerequisites() const
{
	if (!checked_)
		isAvailable();
	return prerequisites_;
}


bool LyXCiteEngine::isAvailable() const
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


bool LyXCiteEngine::hasEngineType(CiteEngineType const & et) const
{
	return std::find(engine_types_.begin(), engine_types_.end(),
			 theCiteEnginesList.getTypeAsString(et)) != engine_types_.end();
}


bool LyXCiteEngine::isCompatible(string const & cename) const
{
	// do we exclude it?
	if (find(excluded_engines_.begin(), excluded_engines_.end(), cename) !=
			excluded_engines_.end())
		return false;

	LyXCiteEngine const * const lm = theCiteEnginesList[cename];
	if (!lm)
		return true;

	// does it exclude us?
	vector<string> const excengs = lm->getExcludedEngines();
	if (find(excengs.begin(), excengs.end(), id_) != excengs.end())
		return false;

	return true;
}


bool LyXCiteEngine::areCompatible(string const & eng1, string const & eng2)
{
	LyXCiteEngine const * const lm1 = theCiteEnginesList[eng1];
	if (lm1)
		return lm1->isCompatible(eng2);
	LyXCiteEngine const * const lm2 = theCiteEnginesList[eng2];
	if (lm2)
		return lm2->isCompatible(eng1);
	// Can't check it either way.
	return true;
}


string LyXCiteEngine::getDefaultBiblio(CiteEngineType const & cet) const
{
	string res;
	string const etp = theCiteEnginesList.getTypeAsString(cet) + ":";
	//check whether all of the required packages are available
	vector<string>::const_iterator it  = default_biblios_.begin();
	vector<string>::const_iterator end = default_biblios_.end();
	for (; it != end; ++it) {
		string const s = *it;
		if (prefixIs(s, etp))
			res = split(s, ':');
		else if (!contains(s, ':') && res.empty())
			res = s;
	}
	return res;
}


bool LyXCiteEngine::isDefaultBiblio(string const & bf) const
{
	string const bfs = ":" + bf;
	vector<string>::const_iterator it  = default_biblios_.begin();
	vector<string>::const_iterator end = default_biblios_.end();
	for (; it != end; ++it) {
		string const s = *it;
		if (suffixIs(s, bfs) || bf == s)
			return true;
	}
	return false;
}


// used when sorting the cite engine list.
class EngineSorter {
public:
	int operator()(LyXCiteEngine const & ce1, LyXCiteEngine const & ce2) const
	{
		return _(ce1.getName()) < _(ce2.getName());
	}
};


// Local translators
namespace {

typedef Translator<string, CiteEngineType> CiteEngineTypeTranslator;


CiteEngineTypeTranslator const init_citeenginetypetranslator()
{
	CiteEngineTypeTranslator translator("authoryear", ENGINE_TYPE_AUTHORYEAR);
	translator.addPair("numerical", ENGINE_TYPE_NUMERICAL);
	translator.addPair("default", ENGINE_TYPE_DEFAULT);
	return translator;
}


CiteEngineTypeTranslator const & citeenginetypetranslator()
{
	static CiteEngineTypeTranslator const translator =
		init_citeenginetypetranslator();
	return translator;
}

} // namespace anon


string CiteEnginesList::getTypeAsString(CiteEngineType const & et) const
{
	return citeenginetypetranslator().find(et);
}


CiteEngineType CiteEnginesList::getType(string const & et) const
{
	return citeenginetypetranslator().find(et);
}


// Much of this is borrowed from LayoutFileList::read()
bool CiteEnginesList::read()
{
	FileName const real_file = libFileSearch("", "lyxciteengines.lst");
	LYXERR(Debug::TCLASS, "Reading cite engines from `" << real_file << '\'');

	if (real_file.empty()) {
		LYXERR0("unable to find cite engines file `citeengines.lst'.\n"
			<< "No cite engines will be available.");
		return false;
	}

	Lexer lex;
	if (!lex.setFile(real_file)) {
		LYXERR0("lyxlex was not able to set file: "
			<< real_file << ".\nNo cite engines will be available.");
		return false;
	}

	if (!lex.isOK()) {
		LYXERR0("unable to open cite engines file  `"
			<< to_utf8(makeDisplayPath(real_file.absFileName(), 1000))
			<< "'\nNo cite engines will be available.");
		return false;
	}

	bool finished = false;
	// Parse cite engines files
	LYXERR(Debug::TCLASS, "Starting parsing of lyxciteengines.lst");
	while (lex.isOK() && !finished) {
		LYXERR(Debug::TCLASS, "\tline by line");
		switch (lex.lex()) {
		case Lexer::LEX_FEOF:
			finished = true;
			break;
		default:
			string const cename = lex.getString();
			LYXERR(Debug::TCLASS, "Engine name: " << cename);
			if (!lex.next())
				break;
			string const fname = lex.getString();
			LYXERR(Debug::TCLASS, "Filename: " << fname);
			if (!lex.next(true))
				break;
			string cet = lex.getString();
			LYXERR(Debug::TCLASS, "Engine Type: " << cet);
			vector<string> cets;
			while (!cet.empty()) {
				string p;
				cet = split(cet, p, '|');
				cets.push_back(p);
			}
			if (!lex.next(true))
				break;
			string db = lex.getString();
			LYXERR(Debug::TCLASS, "Default Biblio: " << db);
			vector<string> dbs;
			while (!db.empty()) {
				string p;
				db = split(db, p, '|');
				dbs.push_back(p);
			}
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
			// This code is run when we have
			// cename, fname, desc, pkgs, req and exc
			addCiteEngine(cename, fname, cets, dbs, desc, pkgs, req, exc);
		} // end switch
	} //end while

	LYXERR(Debug::TCLASS, "End of parsing of lyxciteengines.lst");

	if (!theCiteEnginesList.empty())
		sort(theCiteEnginesList.begin(), theCiteEnginesList.end(), EngineSorter());
	return true;
}


void CiteEnginesList::addCiteEngine(string const & cename,
	string const & filename, vector<string> const & cets,
	vector<string> const & dbs, string const & description,
	vector<string> const & pkgs, vector<string> const & req,
	vector<string> const & exc)
{
	LyXCiteEngine ce(cename, filename, cets, dbs, description, pkgs, req, exc);
	englist_.push_back(ce);
}


LyXCiteEnginesList::const_iterator CiteEnginesList::begin() const
{
	return englist_.begin();
}


LyXCiteEnginesList::iterator CiteEnginesList::begin()
{
	return englist_.begin();
}


LyXCiteEnginesList::const_iterator CiteEnginesList::end() const
{
	return englist_.end();
}


LyXCiteEnginesList::iterator CiteEnginesList::end()
{
	return englist_.end();
}


LyXCiteEngine const * CiteEnginesList::operator[](string const & str) const
{
	LyXCiteEnginesList::const_iterator it = englist_.begin();
	for (; it != englist_.end(); ++it)
		if (it->getID() == str) {
			LyXCiteEngine const & eng = *it;
			return &eng;
		}
	return 0;
}


LyXCiteEngine * CiteEnginesList::operator[](string const & str)
{
	LyXCiteEnginesList::iterator it = englist_.begin();
	for (; it != englist_.end(); ++it)
		if (it->getID() == str) {
			LyXCiteEngine & eng = *it;
			return &eng;
		}
	return 0;
}

} // namespace lyx
