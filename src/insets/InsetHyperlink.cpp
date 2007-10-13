/**
 * \file InsetHyperlink.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
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

#include "support/std_ostream.h"


namespace lyx {

using support::subst;

using std::string;
using std::ostream;


InsetHyperlink::InsetHyperlink(InsetCommandParams const & p)
	: InsetCommand(p, "href")
{}


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
	docstring const & name = getParam("name");
	if (runparams.moving_arg)
		os << "\\protect";
	//set the target for the name when no name is given
	if (!getParam("name").empty())
		os << "\\href{" << getParam("target") << "}{" << getParam("name") << '}';
	else
		os << "\\href{" << getParam("target") << "}{" << getParam("target") << '}';
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
