/**
 * \file IdSc.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "IdSc.h"
#include "support/lstrings.h"

using lyx::support::split;

using std::string;


/// Extract shortcut from "<identifer>|#<shortcut>" string
string const id_sc::shortcut(string const & idsc)
{
	string sc = split(idsc, '|');
	if (!sc.empty() && sc[0] == '#')
		sc.erase(sc.begin());
	return sc;
}


/// Extract identifier from "<identifer>|#<shortcut>" string
string const id_sc::id(string const & idsc)
{
	string id;
	split(idsc, id, '|');
	return id;
}
