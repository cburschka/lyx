/**
 * \file forms_gettext.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */
#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "forms_gettext.h"

#include FORMS_H_LOCATION

// Extract shortcut from <ident>|<shortcut> string
char const * flyx_shortcut_extract(char const * sc)
{
	// Find '|' in the sc and return the string after that.
	register char const * sd = sc;
	while (sd[0]!= 0 && sd[0] != '|') ++sd;

	if (sd[0] == '|') {
		++sd;
		return sd;
	}
	return "";
}


// Extract identifier from <ident>|<shortcut> string
char const * flyx_ident_extract(char const * sc)
{
	register char const * se = sc;
	while (se[0]!= 0 && se[0] != '|') ++se;

	if (se[0] == 0) return sc;

	char * sb = new char[se - sc + 1];
	int index = 0;
	register char const * sd = sc;
	while (sd != se) {
		sb[index] = sd[0];
		++index; ++sd;
	}
	sb[index] = 0;
	return sb;
}
