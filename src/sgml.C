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
#include "outputparams.h"
#include "paragraph.h"

#include "support/lstrings.h"
#include "support/std_ostream.h"
#include "support/convert.h"

#include <boost/tuple/tuple.hpp>

#include <map>
#include <sstream>

using lyx::support::subst;

using std::make_pair;
using std::map;
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


string const uniqueID(string const label)
{
	static unsigned int seed = 1000;
	return label + convert<string>(++seed);
}


string cleanID(Buffer const & buf, OutputParams const & runparams, std::string const & orig)
{
	// The standard DocBook SGML declaration only allows letters,
	// digits, '-' and '.' in a name.
	// Since users might change that declaration one has to cater
	// for additional allowed characters.
	// This routine replaces illegal characters by '-' or '.'
	// and adds a number for uniqueness.
	// If you know what you are doing, you can set allowed==""
	// to disable this mangling.
	LyXTextClass const & tclass = buf.params().getLyXTextClass();
	string const allowed = runparams.flavor == OutputParams::XML? ".-_:":tclass.options();

	if (allowed.empty())
		return orig;

	string::const_iterator it  = orig.begin();
	string::const_iterator end = orig.end();

	string content;

	typedef map<string, string> MangledMap;
	static MangledMap mangledNames;
	static int mangleID = 1;

	MangledMap::const_iterator const known = mangledNames.find(orig);
	if (known != mangledNames.end())
		return (*known).second;

	// make sure it starts with a letter
	if (!isalpha(*it) && allowed.find(*it) >= allowed.size())
		content += "x";

	bool mangle = false;
	for (; it != end; ++it) {
		char c = *it;
		if (isalpha(c) || isdigit(c) || c == '-' || c == '.' || allowed.find(c) < allowed.size())
			content += c;
		else if (c == '_' || c == ' ') {
			mangle = true;
			content += "-";
		}
		else if (c == ':' || c == ',' || c == ';' || c == '!') {
			mangle = true;
			content += ".";
		}
		else {
			mangle = true;
		}
	}
	if (mangle) {
		content += "-" + convert<string>(mangleID++);
	}
	else if (isdigit(content[content.size() - 1])) {
		content += ".";
	}

	mangledNames[orig] = content;

	return content;
}


void openTag(ostream & os, string const & name, string const & attribute)
{
	// This should be fixed in layout files later.
	string param = subst(attribute, "<", "\"");
	param = subst(param, ">", "\"");

	if (!name.empty() && name != "!-- --") {
		os << '<' << name;
		if (!param.empty())
			os << " " << param;
		os << '>';
	}
}


void closeTag(ostream & os, string const & name)
{
	if (!name.empty() && name != "!-- --")
		os << "</" << name << '>';
}


void openTag(Buffer const & buf, ostream & os, OutputParams const & runparams, Paragraph const & par)
{
	LyXLayout_ptr const & style = par.layout();
	string const & name = style->latexname();
	string param = style->latexparam();
	Counters & counters = buf.params().getLyXTextClass().counters();

	string id = par.getID(buf, runparams);

	string attribute;
	if(!id.empty()) {
		if (param.find('#') != string::npos) {
			string::size_type pos = param.find("id=<");
			string::size_type end = param.find(">");
			if( pos != string::npos && end != string::npos)
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
			attribute = subst(param, "#", convert<string>(i));
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
