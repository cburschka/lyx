/**
 * \file InsetHyperlink.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetHyperlink.h"

#include "DispatchResult.h"
#include "FuncRequest.h"
#include "LaTeXFeatures.h"
#include "gettext.h"
#include "OutputParams.h"

#include "support/lstrings.h"
#include "support/docstream.h"


namespace lyx {

using support::subst;

using std::string;
using std::ostream;
using std::find;
using std::replace;

//FIXME: these should be lists of char_type and not char
static char_type const chars_url[2] = {'%', '#'};

static char_type const chars_name[6] = {
	'&', '_', '$', '%', '#', '^'};


InsetHyperlink::InsetHyperlink(InsetCommandParams const & p)
	: InsetCommand(p, "href")
{}


CommandInfo const * InsetHyperlink::findInfo(std::string const & /* cmdName */)
{
	static const char * const paramnames[] =
		{"name", "target", "type", ""};
	static const bool isoptional[] = {true, false};
	static const CommandInfo info = {3, paramnames, isoptional};
	return &info;
}


docstring const InsetHyperlink::getScreenLabel(Buffer const &) const
{
	docstring const temp = from_ascii("Hyperlink: ");

	docstring url;

	url += getParam("name");
	if (url.empty())
		url += getParam("target");

	// elide if long
	if (url.length() > 30) {
		url = url.substr(0, 10) + "..."
			+ url.substr(url.length() - 17, url.length());
	}
	return temp + url;
}


int InsetHyperlink::latex(Buffer const &, odocstream & os,
		    OutputParams const & runparams) const
{
	//FIXME: all strings in this routine should be docstrings
	docstring url = getParam("target");

	docstring backslash = from_ascii("\\");
	docstring braces = from_ascii("{}");

	// The characters in chars_url[] need to be changed to a command when
	// they are in the url field.
	if (!url.empty()) {
		// the chars_url[] characters must be handled for both, url and href
		for (int k = 0;	k < 2; k++) {
			for (size_t i = 0, pos;
				(pos = url.find(chars_url[k], i)) != string::npos;
				i = pos + 2) {
				url.replace(pos, 1, backslash + chars_url[k]);
			}
		}
	} // end if (!url.empty())

	docstring name = getParam("name");

	// The characters in chars_name[] need to be changed to a command when
	// they are in the name field.
	if (!name.empty()) {

		// handle the "\" character, but only when the following character
		// is not also a "\", because "\\" is valid code
		docstring textbackslash = from_ascii("\\textbackslash{}");
		for (size_t i = 0, pos;
			(pos = name.find('\\', i)) != string::npos;
			i = pos + 2) {
			if	(name[pos + 1] != '\\')
				name.replace(pos, 1, textbackslash);
		}
		for (int k = 0;	k < 6; k++) {
			for (size_t i = 0, pos;
				(pos = name.find(chars_name[k], i)) != string::npos;
				i = pos + 2) {
				name.replace(pos, 1, backslash + chars_name[k] + braces);
			}
		}
		// replace the tilde by the \sim character as suggested in the LaTeX FAQ
		// for URLs
		docstring sim = from_ascii("$\\sim$");
		for (int i = 0, pos;
			(pos = name.find('~', i)) != string::npos;
			i = pos + 1)
			name.replace(pos, 1, sim);

	}  // end if (!name.empty())
	
	//for the case there is no name given, the target is set as name
	docstring urlname = url;
	// set the hyperlink type
	url += getParam("type");

	if (runparams.moving_arg)
		os << "\\protect";
	//set the target for the name when no name is given
	if (!name.empty())
		os << "\\href{" << url << "}{" << name << '}';
	else
		os << "\\href{" << url << "}{" << urlname << '}';
	return 0;
}


int InsetHyperlink::plaintext(Buffer const &, odocstream & os,
			OutputParams const &) const
{
	odocstringstream oss;

	oss << '[' << getParam("target");
	if (getParam("name").empty())
		oss << ']';
	else
		oss << "||" << getParam("name") << ']';

	docstring const str = oss.str();
	os << str;
	return str.size();
}


int InsetHyperlink::docbook(Buffer const &, odocstream & os,
		      OutputParams const &) const
{
	os << "<ulink url=\""
	   << subst(getParam("target"), from_ascii("&"), from_ascii("&amp;"))
	   << "\">"
	   << getParam("name")
	   << "</ulink>";
	return 0;
}


int InsetHyperlink::textString(Buffer const & buf, odocstream & os,
		       OutputParams const & op) const
{
	return plaintext(buf, os, op);
}


void InsetHyperlink::validate(LaTeXFeatures & features) const
{
	features.require("hyperref");
}


} // namespace lyx
