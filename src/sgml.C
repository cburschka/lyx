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

#include "sgml.h"

#include "buffer.h"
#include "bufferparams.h"
#include "counters.h"
#include "lyxtext.h"
#include "paragraph.h"

#include "support/lstrings.h"
#include "support/std_ostream.h"
#include "support/tostr.h"

using lyx::support::subst;

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
#if 0
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
#endif
	default:
		str = c;
		break;
	}
	return make_pair(false, str);
}


int openTag(Buffer const & buf, ostream & os, Paragraph::depth_type depth,
	    bool mixcont, string const & name, string const & param)
{
	Counters & counters = buf.params().getLyXTextClass().counters();
	LyXLayout_ptr const & defaultstyle = buf.params().getLyXTextClass().defaultLayout();

	string attribute = param;
	// code for paragraphs like the standard paragraph in AGU.
	if ( defaultstyle->latexname() == name and !defaultstyle->latexparam().empty()) {
		counters.step(name);
		int i = counters.value(name);
		attribute += "" + subst(defaultstyle->latexparam(), "#", tostr(i));
	}

	if (!name.empty() && name != "!-- --") {
		if (!mixcont)
			os << string(depth, ' ');
		os << '<' << name;
		if (!attribute.empty())
			os << " " << attribute;
		os << '>';
	}

	return !mixcont;
}


int closeTag(ostream & os, Paragraph::depth_type depth,
	     bool mixcont, string const & name)
{
	if (!name.empty() && name != "!-- --") {
		if (!mixcont)
			os << '\n' << string(depth, ' ');
		os << "</" << name << '>';
	}

	if (!mixcont)
		os << '\n';

	return !mixcont;
}

} // namespace sgml
