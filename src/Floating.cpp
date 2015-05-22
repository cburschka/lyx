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

#include "support/debug.h"
#include "support/lstrings.h"
#include "support/textutils.h"

using namespace std;


namespace lyx {


Floating::Floating(string const & type, string const & placement,
		   string const & ext, string const & within,
		   string const & style, string const & name,
		   string const & listName, std::string const & listCmd,
		   string const & refPrefix, std::string const & allowedplacement,
		   string const & htmlTag, string const & htmlAttrib,
		   string const & htmlStyle, bool usesfloat, bool ispredefined)
	: floattype_(type), placement_(placement), ext_(ext), within_(within),
	  style_(style), name_(name), listname_(listName), listcommand_(listCmd),
	  refprefix_(refPrefix), allowedplacement_(allowedplacement),
	  usesfloatpkg_(usesfloat), ispredefined_(ispredefined),
	  html_tag_(htmlTag), html_attrib_(htmlAttrib), html_style_(htmlStyle)
{}


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
	string n = floattype_;
	string::const_iterator it = n.begin();
	string::const_iterator en = n.end();
	for (; it != en; ++it) {
		if (!isAlphaASCII(*it))
			d += "_";
		else if (isLower(*it))
			d += *it;
		else
			d += support::lowercase(*it);
	}
	// are there other characters we need to remove?
	defaultcssclass_ = "float-" + d;
	return defaultcssclass_;
}


} // namespace lyx
