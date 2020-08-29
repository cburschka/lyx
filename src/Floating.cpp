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
		   docstring const & htmlStyle, string const & docbookTag,
		   string const & docbookAttr, string const & docbookTagType,
           string const & required, bool usesfloat, bool ispredefined,
		   bool allowswide, bool allowssideways)
	: floattype_(type), placement_(placement), ext_(ext), within_(within),
	  style_(style), name_(name), listname_(listName), listcommand_(listCmd),
	  refprefix_(refPrefix), allowedplacement_(allowedplacement), required_(required),
	  usesfloatpkg_(usesfloat), ispredefined_(ispredefined),
	  allowswide_(allowswide), allowssideways_(allowssideways),
	  html_tag_(htmlTag), html_attrib_(htmlAttrib), html_style_(htmlStyle),
	  docbook_attr_(docbookAttr), docbook_tag_type_(docbookTagType)
{}


string const & Floating::htmlAttrib() const
{
	if (html_attrib_.empty())
		html_attrib_ = "class='" + defaultCSSClass() + "'";
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


string const & Floating::docbookAttr() const
{
	return docbook_attr_;
}


string Floating::docbookTag(bool hasTitle) const
{
	if (floattype_ == "figure") {
		return hasTitle ? "figure" : "informalfigure";
	} else if (floattype_ == "table") {
		return hasTitle ? "table" : "informaltable";
	} else if (floattype_ == "algorithm") {
		// TODO: no good translation for now! Figures are the closest match, as they can contain text.
		// Solvable as soon as https://github.com/docbook/docbook/issues/157 has a definitive answer.
		return "figure";
	}
}


string const & Floating::docbookTagType() const
{
	if (docbook_tag_type_ != "block" && docbook_tag_type_ != "paragraph" && docbook_tag_type_ != "inline")
		docbook_tag_type_ = "block";
	return docbook_tag_type_;
}


string const & Floating::docbookCaption() const
{
	docbook_caption_ = "";
	if (floattype_ == "figure") {
		docbook_caption_ = "title";
	} else if (floattype_ == "table") {
		docbook_caption_ = "caption";
	} else if (floattype_ == "algorithm") {
		// TODO: no good translation for now! Figures are the closest match, as they can contain text.
		// Solvable as soon as https://github.com/docbook/docbook/issues/157 has a definitive answer.
		docbook_caption_ = "title";
	}
	return docbook_caption_;
}


} // namespace lyx
