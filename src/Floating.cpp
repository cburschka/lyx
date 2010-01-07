/**
 * \file Floating.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Floating.h"

#include "support/lstrings.h"

using namespace std;


namespace lyx {


Floating::Floating()
{}


Floating::Floating(string const & type, string const & placement,
		   string const & ext, string const & within,
		   string const & style, string const & name,
		   string const & listName, string const & htmlTag,
		   string const & htmlAttrib, string const & htmlStyle,
		   bool builtin)
	: type_(type), placement_(placement), ext_(ext), within_(within),
	  style_(style), name_(name), listName_(listName), html_tag_(htmlTag),
	  html_attrib_(htmlAttrib), html_style_(htmlStyle), builtin_(builtin)
{}


string const & Floating::type() const
{
	return type_;
}


string const & Floating::placement() const
{
	return placement_;
}


string const & Floating::ext() const
{
	return ext_;
}


string const & Floating::within() const
{
	return within_;
}


string const & Floating::style() const
{
	return style_;
}


string const & Floating::name() const
{
	return name_;
}


string const & Floating::listName() const
{
	return listName_;
}


string const & Floating::htmlStyle() const
{
	return html_style_;
}


string const & Floating::htmlAttrib() const
{
	if (html_attrib_.empty())
		html_attrib_ = "class='float " + defaultCSSClass() + "'";
	return html_attrib_;
}


string const & Floating::htmlTag() const
{
	if (html_tag_.empty())
		html_tag_ = "div";
	return html_tag_;
}


string Floating::defaultCSSClass() const
{ 
	if (!defaultcssclass_.empty())
		return defaultcssclass_;
	string d;
	string n = type_;
	string::const_iterator it = n.begin();
	string::const_iterator en = n.end();
	for (; it != en; ++it) {
		if (!isalpha(*it))
			d += "_";
		else if (islower(*it))
			d += *it;
		else
			d += support::lowercase(*it);
	}
	// are there other characters we need to remove?
	defaultcssclass_ = "float-" + d;
	return defaultcssclass_;
}


bool Floating::builtin() const
{
	return builtin_;
}


} // namespace lyx
