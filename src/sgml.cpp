/**
 * \file sgml.cpp
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

#include "Buffer.h"
#include "BufferParams.h"
#include "Counters.h"
#include "Layout.h"
#include "OutputParams.h"
#include "Paragraph.h"
#include "Text.h"
#include "TextClass.h"

#include "support/convert.h"
#include "support/docstream.h"
#include "support/lstrings.h"
#include "support/textutils.h"
#include "support/ThreadStorage.h"

#include <map>

using namespace std;
using namespace lyx::support;

namespace lyx {


docstring sgml::escapeChar(char_type c)
{
	docstring str;
	switch (c) {
	case ' ':
		str += " ";
		break;
	case '&':
		str += "&amp;";
		break;
	case '<':
		str += "&lt;";
		break;
	case '>':
		str += "&gt;";
		break;
#if 0
	case '$':
		str += "&dollar;";
		break;
	case '#':
		str += "&num;";
		break;
	case '%':
		str += "&percnt;";
		break;
	case '[':
		str += "&lsqb;";
		break;
	case ']':
		str += "&rsqb;";
		break;
	case '{':
		str += "&lcub;";
		break;
	case '}':
		str += "&rcub;";
		break;
	case '~':
		str += "&tilde;";
		break;
	case '"':
		str += "&quot;";
		break;
	case '\\':
		str += "&bsol;";
		break;
#endif
	default:
		str += c;
		break;
	}
	return str;
}


docstring sgml::escapeString(docstring const & raw)
{
	docstring bin;
	bin.reserve(raw.size() * 2); // crude approximation is sufficient
	for (size_t i = 0; i != raw.size(); ++i)
		bin += sgml::escapeChar(raw[i]);

	return bin;
}


docstring const sgml::uniqueID(docstring const & label)
{
	// FIXME THREAD
	// It seems unlikely there could be a problem here,
	// but we could have concurrent access, in principle.
	static unsigned int seed = 1000;
	return label + convert<docstring>(++seed);
}


docstring sgml::cleanID(Buffer const & buf, OutputParams const & runparams,
	docstring const & orig)
{
	// The standard DocBook SGML declaration only allows letters,
	// digits, '-' and '.' in a name.
	// Since users might change that declaration one has to cater
	// for additional allowed characters.
	// This routine replaces illegal characters by '-' or '.'
	// and adds a number for uniqueness.
	// If you know what you are doing, you can set allowed==""
	// to disable this mangling.
	DocumentClass const & tclass = buf.params().documentClass();
	docstring const allowed = from_ascii(
		runparams.flavor == OutputParams::XML ? ".-_:" : tclass.options());

	if (allowed.empty())
		return orig;

	docstring::const_iterator it  = orig.begin();
	docstring::const_iterator end = orig.end();

	docstring content;

	typedef map<docstring, docstring> MangledMap;
	static ThreadStorage<MangledMap> tMangledNames;
	MangledMap & mangledNames = *tMangledNames;
	static ThreadStorage<int> tMangleID;
	int & mangleID = *tMangleID;

	MangledMap::const_iterator const known = mangledNames.find(orig);
	if (known != mangledNames.end())
		return known->second;

	// make sure it starts with a letter
	if (!isAlphaASCII(*it) && allowed.find(*it) >= allowed.size())
		content += "x";

	bool mangle = false;
	for (; it != end; ++it) {
		char_type c = *it;
		if (isAlphaASCII(c) || isDigitASCII(c) || c == '-' || c == '.'
		      || allowed.find(c) < allowed.size())
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

	if (mangle)
		content += "-" + convert<docstring>(mangleID++);
	else if (isDigitASCII(content[content.size() - 1]))
		content += ".";

	mangledNames[orig] = content;

	return content;
}


void sgml::openTag(odocstream & os, string const & name, string const & attribute)
{
	// FIXME UNICODE
	// This should be fixed in layout files later.
	string param = subst(attribute, "<", "\"");
	param = subst(param, ">", "\"");

	// Note: we ignore the name if it empty or if it is a comment "<!-- -->" or
	// if the name is *dummy*.
	// We ignore dummy because dummy is not a valid docbook element and it is
	// the internal name given to single paragraphs in the latex output.
	// This allow us to simplify the code a lot and is a reasonable compromise.
	if (!name.empty() && name != "!-- --" && name != "dummy") {
		os << '<' << from_ascii(name);
		if (!param.empty())
			os << ' ' << from_ascii(param);
		os << '>';
	}
}


void sgml::closeTag(odocstream & os, string const & name)
{
	if (!name.empty() && name != "!-- --" && name != "dummy")
		os << "</" << from_ascii(name) << '>';
}


void sgml::openTag(Buffer const & buf, odocstream & os,
	OutputParams const & runparams, Paragraph const & par)
{
	Layout const & style = par.layout();
	string const & name = style.latexname();
	string param = style.latexparam();
	Counters & counters = buf.params().documentClass().counters();

	string id = par.getID(buf, runparams);

	string attribute;
	if (!id.empty()) {
		if (param.find('#') != string::npos) {
			string::size_type pos = param.find("id=<");
			string::size_type end = param.find(">");
			if( pos != string::npos && end != string::npos)
				param.erase(pos, end-pos + 1);
		}
		attribute = id + ' ' + param;
	} else {
		if (param.find('#') != string::npos) {
			// FIXME UNICODE
			if (!style.counter.empty())
				// This uses InternalUpdate at the moment becuase sgml output
				// does not do anything with tracked counters, and it would need
				// to track layouts if it did want to use them.
				counters.step(style.counter, InternalUpdate);
			else
				counters.step(from_ascii(name), InternalUpdate);
			int i = counters.value(from_ascii(name));
			attribute = subst(param, "#", convert<string>(i));
		} else {
			attribute = param;
		}
	}
	openTag(os, name, attribute);
}


void sgml::closeTag(odocstream & os, Paragraph const & par)
{
	Layout const & style = par.layout();
	closeTag(os, style.latexname());
}


} // namespace lyx
