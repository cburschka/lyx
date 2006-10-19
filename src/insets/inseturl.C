/**
 * \file inseturl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "inseturl.h"

#include "dispatchresult.h"
#include "funcrequest.h"
#include "LaTeXFeatures.h"
#include "gettext.h"
#include "outputparams.h"

#include "support/lstrings.h"

#include "support/std_ostream.h"

using lyx::docstring;
using lyx::odocstream;
using lyx::support::subst;

using std::string;
using std::ostream;


InsetUrl::InsetUrl(InsetCommandParams const & p)
	: InsetCommand(p, "url")
{}


docstring const InsetUrl::getScreenLabel(Buffer const &) const
{
	docstring const temp =
		(getCmdName() == "url") ? _("Url: ") : _("HtmlUrl: ");

	string url;

	if (!getOptions().empty())
		url += getOptions();
	else
		url += getContents();

	// elide if long
	if (url.length() > 30) {
		url = url.substr(0, 10) + "..."
			+ url.substr(url.length() - 17, url.length());
	}
	// FIXME UNICODE
	return temp + lyx::from_utf8(url);
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
	// FIXME UNICODE
	os << '[' << lyx::from_utf8(getContents());
	if (getOptions().empty())
		os << ']';
	else
		// FIXME UNICODE
		os << "||" << lyx::from_utf8(getOptions()) << ']';
	return 0;
}


int InsetUrl::docbook(Buffer const &, ostream & os,
		      OutputParams const &) const
{
	os << "<ulink url=\"" << subst(getContents(),"&","&amp;")
	   << "\">" << getOptions() << "</ulink>";
	return 0;
}


int InsetUrl::textString(Buffer const & buf, odocstream & os,
		       OutputParams const & op) const
{
	return plaintext(buf, os, op);
}


void InsetUrl::validate(LaTeXFeatures & features) const
{
	features.require("url");
}
