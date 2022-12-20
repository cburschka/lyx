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
#include "support/debug.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/lstrings.h"


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

	// required date as int (yyyymmdd)
	int const req_date = (y * 10000) + (m * 100) + d;
	for (auto const & package : packages_) {
		if (package.first == name && !package.second.empty()) {
			if (!isStrInt(package.second)) {
				LYXERR0("Warning: Invalid date of package "
					<< package.first << " (" << package.second << ")");
				continue;
			}
			// required date not newer than available date
			return req_date <= convert<int>(package.second);
		}
	}
	return false;
}

} // namespace lyx
