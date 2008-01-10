/**
 * \file InsetUrl.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetUrl.h"

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


InsetUrl::InsetUrl(InsetCommandParams const & p)
	: InsetCommand(p, "url")
{}


docstring const InsetUrl::getScreenLabel(Buffer const &) const
{
	docstring const temp =
		(getCmdName() == "url") ? _("Url: ") : _("HtmlUrl: ");

	docstring url;

	if (!getParam("name").empty())
		url += getParam("name");
	else
		url += getParam("target");

	// elide if long
	if (url.length() > 30) {
		url = url.substr(0, 10) + "..."
			+ url.substr(url.length() - 17, url.length());
	}
	return temp + url;
}


int InsetUrl::latex(Buffer const &, odocstream & os,
		    OutputParams const & runparams) const
{
	docstring const & name = getParam("name");
	if (!name.empty())
		os << name + ' ';
	if (runparams.moving_arg)
		os << "\\protect";
	os << "\\url{" << getParam("target") << '}';
	return 0;
}


int InsetUrl::plaintext(Buffer const &, odocstream & os,
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


int InsetUrl::docbook(Buffer const &, odocstream & os,
		      OutputParams const &) const
{
	os << "<ulink url=\""
	   << subst(getParam("target"), from_ascii("&"), from_ascii("&amp;"))
	   << "\">"
	   << getParam("name")
	   << "</ulink>";
	return 0;
}


void InsetUrl::textString(Buffer const & buf, odocstream & os) const
{
	plaintext(buf, os, OutputParams(0));
}


void InsetUrl::validate(LaTeXFeatures & features) const
{
	features.require("url");
}


} // namespace lyx
