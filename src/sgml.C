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

#include <boost/tuple/tuple.hpp>

#include <sstream>

using lyx::support::subst;

using std::make_pair;

using std::ostream;
using std::ostringstream;
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


string escapeString(string const & raw)
{
	ostringstream bin;

	for(string::size_type i = 0; i < raw.size(); ++i) {
		bool ws;
		string str;
		boost::tie(ws, str) = sgml::escapeChar(raw[i]);
		bin << str;
	}
	return bin.str();
}


void openTag(ostream & os, string const & name, string const & attribute)
{
	if (!name.empty() && name != "!-- --") {
		os << '<' << name;
		if (!attribute.empty())
			os << " " << attribute;
		os << '>';
	}
}


void closeTag(ostream & os, string const & name)
{
	if (!name.empty() && name != "!-- --")
		os << "</" << name << '>';
}


void openTag(Buffer const & buf, ostream & os, Paragraph const & par)
{
	LyXLayout_ptr const & style = par.layout();
	string const & name = style->latexname();
	string param = style->latexparam();
	Counters & counters = buf.params().getLyXTextClass().counters();

	string id = par.getDocbookId();
	id = id.empty()? "" : " id = \"" + id + "\"";

	string attribute;
	if(!id.empty()) {
		if (param.find('#') != string::npos) {
			string::size_type pos = param.find("id=<");
			string::size_type end = param.find(">");
			if( pos != string::npos and end != string::npos)
				param.erase(pos, end-pos + 1);
		}
		attribute = id + ' ' + param;
	} else {
		if (param.find('#') != string::npos) {
			if(!style->counter.empty())
				counters.step(style->counter);
			else
				counters.step(style->latexname());
			int i = counters.value(name);
			attribute = subst(param, "#", tostr(i));
			attribute = subst(attribute, "<", "\"");
			attribute = subst(attribute, ">", "\"");
		} else {
			attribute = param;
		}
	}
	openTag(os, name, attribute);
}


void closeTag(ostream & os, Paragraph const & par)
{
	LyXLayout_ptr const & style = par.layout();
	closeTag(os, style->latexname());
}

} // namespace sgml
