/**
 * \file LaTeXPackages.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Jürgen Vigna
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LaTeXPackages.h"

#include "Lexer.h"

#include "support/convert.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/lstrings.h"

#include <regex>


using namespace std;
using namespace lyx::support;


namespace lyx {

LaTeXPackages::Packages LaTeXPackages::packages_;


void LaTeXPackages::getAvailable()
{
	Lexer lex;
	support::FileName const real_file = libFileSearch("", "packages.lst");

	if (real_file.empty())
		return;

	lex.setFile(real_file);

	if (!lex.isOK())
		return;

	// Make sure that we are clean
	packages_.clear();

	bool finished = false;
	// Parse config-file
	while (lex.isOK() && !finished) {
		switch (lex.lex()) {
		case Lexer::LEX_FEOF:
			finished = true;
			break;
		default: {
			string const p = lex.getString();
			// Parse optional version info
			lex.eatLine();
			string const v = trim(lex.getString());
			packages_.insert(make_pair(p, v));
		}
		}
	}
}


bool LaTeXPackages::isAvailable(string const & name)
{
	if (packages_.empty())
		getAvailable();
	string n = name;
	if (suffixIs(n, ".sty"))
		n.erase(name.length() - 4);
	for (auto const & package : packages_) {
		if (package.first == n)
			return true;
	}
	return false;
}


bool LaTeXPackages::isAvailableAtLeastFrom(string const & name,
					   int const y, int const m, int const d)
{
	if (packages_.empty())
		getAvailable();

	bool result = false;
	// Check for yyyy[-/]mm[-/]dd
	static regex const reg("([\\d]{4})[-/]?([\\d]{2})[-/]?([\\d]{2})");
	for (auto const & package : packages_) {
		if (package.first == name && !package.second.empty()) {
			smatch sub;
			if (regex_match(package.second, sub, reg)) {
				// Test whether date is same or newer.
				//
				// Test for year first
				int const avail_y = convert<int>(sub.str(1));
				if (avail_y < y)
					// our year is older: bad!
					break;
				if (avail_y > y) {
					// our year is newer: good!
					result = true;
					break;
				}
				// Same year: now test month
				int const avail_m = convert<int>(sub.str(2));
				if (avail_m < m)
					// our month is older: bad!
					break;
				if (avail_m > m) {
					// our month is newer: good!
					result = true;
					break;
				}
				// Same year and month: test day
				if (convert<int>(sub.str(3)) >= d) {
					// day same or newer: good!
					result = true;
					break;
				}
			}
		}
	}
	return result;
}

} // namespace lyx
