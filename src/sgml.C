/**
 * \file sgml.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/std_ostream.h"

#include "paragraph.h"
#include "sgml.h"

using std::endl;
using std::make_pair;

using std::ostream;
using std::pair;
using std::string;


namespace sgml {

pair<bool, string> escapeChar(char c)
{
	string str;

	switch (c) {
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


int openTag(ostream & os, Paragraph::depth_type depth,
	    bool mixcont, string const & latexname)
{
	if (!latexname.empty() && latexname != "!-- --") {
		if (!mixcont)
			os << string(depth, ' ');
		os << '<' << latexname << '>';
	}

	if (!mixcont)
		os << endl;

	return !mixcont;
}


int closeTag(ostream & os, Paragraph::depth_type depth,
	     bool mixcont, string const & latexname)
{
	if (!latexname.empty() && latexname != "!-- --") {
		if (!mixcont)
			os << endl << string(depth, ' ');
		os << "</" << latexname << '>';
	}

	if (!mixcont)
		os << endl;

	return !mixcont;
}


unsigned int closeEnvTags(ostream & ofs, bool mixcont,
			string const & environment_inner_depth,
			lyx::depth_type total_depth)
{
	unsigned int lines;
	if (environment_inner_depth != "!-- --") {
		string item_name= "listitem";
		lines += closeTag(ofs, total_depth, mixcont, item_name);
		if (environment_inner_depth == "varlistentry")
			lines += closeTag(ofs, total_depth, mixcont,
				environment_inner_depth);
	}
	return lines;
}


} // namespace sgml
