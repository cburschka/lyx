#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "inseturl.h"
#include "BufferView.h"
#include "LaTeXFeatures.h"
#include "LyXView.h"
#include "debug.h"
#include "frontends/Dialogs.h"

using std::ostream;


InsetUrl::InsetUrl(InsetCommandParams const & p)
		: InsetCommand(p)
{}


void InsetUrl::Edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getDialogs()->showUrl( this );
}


string const InsetUrl::getScreenLabel() const
{
	string temp;
	if( getCmdName() == "url" )
		temp = _("Url: ");
	else 
		temp = _("HtmlUrl: ");

	if(!getOptions().empty())
		temp += getOptions();
	else
		temp += getContents();

	return temp;
}


int InsetUrl::Latex(Buffer const *, ostream & os,
		    bool fragile, bool /*free_spc*/) const
{
	if (!getOptions().empty())
		os << getOptions() + ' ';
	if (fragile)
		os << "\\protect";
	os << "\\url{" << getContents() << '}';
	return 0;
}


int InsetUrl::Ascii(Buffer const *, ostream & os, int linelen) const
{
	if (getOptions().empty())
		os << "[" << getContents() << "]";
	else
		os << "[" << getContents() << "||" <<  getOptions() << "]";
	return 0;
}


int InsetUrl::Linuxdoc(Buffer const *, ostream & os) const
{
	os << "<" << getCmdName()
	   << " url=\""  << getContents() << "\""
	   << " name=\"" << getOptions() << "\">";

	return 0;
}


int InsetUrl::DocBook(Buffer const *, ostream & os) const
{
	os << "<ulink url=\"" << getContents() << "\">"
	   << getOptions() << "</ulink>";
	return 0;
}


void InsetUrl::Validate(LaTeXFeatures & features) const
{
	features.url = true;
}
