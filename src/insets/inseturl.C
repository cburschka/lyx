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

using lyx::support::subst;

using std::string;
using std::ostream;


InsetUrl::InsetUrl(InsetCommandParams const & p)
	: InsetCommand(p, "url")
{}


string const InsetUrl::getScreenLabel(Buffer const &) const
{
	string temp;
	if (getCmdName() == "url")
		temp = _("Url: ");
	else
		temp = _("HtmlUrl: ");

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
	return temp + url;
}


int InsetUrl::latex(Buffer const &, ostream & os,
		    OutputParams const & runparams) const
{
	if (!getOptions().empty())
		os << getOptions() + ' ';
	if (runparams.moving_arg)
		os << "\\protect";
	os << "\\url{" << getContents() << '}';
	return 0;
}


int InsetUrl::plaintext(Buffer const &, ostream & os,
		    OutputParams const &) const
{
	if (getOptions().empty())
		os << '[' << getContents() << ']';
	else
		os << '[' << getContents() << "||" <<  getOptions() << ']';
	return 0;
}


int InsetUrl::linuxdoc(Buffer const &, ostream & os,
		       OutputParams const &) const
{
	os << '<' << getCmdName()
	   << " url=\""  << getContents() << "\""
	   << " name=\"" << getOptions() << "\">";

	return 0;
}


int InsetUrl::docbook(Buffer const &, ostream & os,
		      OutputParams const &) const
{
	os << "<ulink url=\"" << subst(getContents(),"&","&amp;")
	   << "\">" << getOptions() << "</ulink>";
	return 0;
}


void InsetUrl::validate(LaTeXFeatures & features) const
{
	features.require("url");
}
