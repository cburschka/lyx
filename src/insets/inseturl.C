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
#include "latexrunparams.h"
#include "LaTeXFeatures.h"
#include "gettext.h"

#include "support/lstrings.h"

#include "support/std_ostream.h"

using lyx::support::subst;

using std::string;
using std::ostream;


InsetUrl::InsetUrl(InsetCommandParams const & p)
		: InsetCommand(p)
{}


// InsetUrl::InsetUrl(InsetCommandParams const & p, bool)
//		: InsetCommand(p, false)
// {}


InsetUrl::~InsetUrl()
{
	InsetCommandMailer("url", *this).hideDialog();
}


DispatchResult
InsetUrl::priv_dispatch(FuncRequest const & cmd,
			idx_type & idx, pos_type & pos)
{
	switch (cmd.action) {
		case LFUN_INSET_EDIT:
			InsetCommandMailer("url", *this).showDialog(cmd.view());
			return DispatchResult(DISPATCHED);
		default:
			return InsetCommand::priv_dispatch(cmd, idx, pos);
	}
}


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
		    LatexRunParams const & runparams) const
{
	if (!getOptions().empty())
		os << getOptions() + ' ';
	if (runparams.moving_arg)
		os << "\\protect";
	os << "\\url{" << getContents() << '}';
	return 0;
}


int InsetUrl::ascii(Buffer const &, ostream & os, int) const
{
	if (getOptions().empty())
		os << '[' << getContents() << ']';
	else
		os << '[' << getContents() << "||" <<  getOptions() << ']';
	return 0;
}


int InsetUrl::linuxdoc(Buffer const &, ostream & os) const
{
	os << '<' << getCmdName()
	   << " url=\""  << getContents() << "\""
	   << " name=\"" << getOptions() << "\">";

	return 0;
}


int InsetUrl::docbook(Buffer const &, ostream & os, bool) const
{
	os << "<ulink url=\"" << subst(getContents(),"&","&amp;")
	   << "\">" << getOptions() << "</ulink>";
	return 0;
}


void InsetUrl::validate(LaTeXFeatures & features) const
{
	features.require("url");
}
