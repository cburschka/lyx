/**
 * \file sgml.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author José Matos
 * \author John Levon <levon@movementarian.org>
 */

#include "sgml.h"
#include "paragraph.h"
 
using std::pair;
using std::make_pair;
 
namespace sgml {

pair<bool, string> escapeChar(char c)
{
	string str;
 
	switch (c) {
	case Paragraph::META_HFILL:
		break;
	case Paragraph::META_NEWLINE:
		str = '\n';
		break;
	case ' ':
		return make_pair(true, string(" "));
		break;
	case '\0': // Ignore :-)
		str.erase();
		break;
	case '&':
		str = "&amp;";
		break;
	case '<':
		str = "&lt;";
		break;
	case '>':
		str = "&gt;";
		break;
	case '$':
		str = "&dollar;";
		break;
	case '#':
		str = "&num;";
		break;
	case '%':
		str = "&percnt;";
		break;
	case '[':
		str = "&lsqb;";
		break;
	case ']':
		str = "&rsqb;";
		break;
	case '{':
		str = "&lcub;";
		break;
	case '}':
		str = "&rcub;";
		break;
	case '~':
		str = "&tilde;";
		break;
	case '"':
		str = "&quot;";
		break;
	case '\\':
		str = "&bsol;";
		break;
	default:
		str = c;
		break;
	}
	return make_pair(false, str);
}

} // namespace sgml
