/**
 * \file inseturl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS
 */
#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "inseturl.h"
#include "BufferView.h"
#include "LaTeXFeatures.h"
#include "frontends/LyXView.h"
#include "debug.h"
#include "frontends/Dialogs.h"
#include "support/lstrings.h"
#include "gettext.h"

using std::ostream;


InsetUrl::InsetUrl(InsetCommandParams const & p, bool)
		: InsetCommand(p)
{}


void InsetUrl::edit(BufferView * bv, int, int, mouse_button::state)
{
	bv->owner()->getDialogs().showUrl(this);
}


void InsetUrl::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, mouse_button::none);
}


string const InsetUrl::getScreenLabel(Buffer const *) const
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


int InsetUrl::latex(Buffer const *, ostream & os,
		    bool fragile, bool /*free_spc*/) const
{
	if (!getOptions().empty())
		os << getOptions() + ' ';
	if (fragile)
		os << "\\protect";
	os << "\\url{" << getContents() << '}';
	return 0;
}


int InsetUrl::ascii(Buffer const *, ostream & os, int) const
{
	if (getOptions().empty())
		os << "[" << getContents() << "]";
	else
		os << "[" << getContents() << "||" <<  getOptions() << "]";
	return 0;
}


int InsetUrl::linuxdoc(Buffer const *, ostream & os) const
{
	os << "<" << getCmdName()
	   << " url=\""  << getContents() << "\""
	   << " name=\"" << getOptions() << "\">";

	return 0;
}


int InsetUrl::docbook(Buffer const *, ostream & os, bool) const
{
	os << "<ulink url=\"" << subst(getContents(),"&","&amp;")
	   << "\">" << getOptions() << "</ulink>";
	return 0;
}


void InsetUrl::validate(LaTeXFeatures & features) const
{
	features.require("url");
}
