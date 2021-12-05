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
#include <set>

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
		   docstring const & htmlStyle, std::string const & docbookTag,
		   string const & docbookAttr, string const & docbookTagType,
           std::string const & docbookFloatType, std::string const & docbookCaption,
           string const & required, bool usesfloat, bool ispredefined,
		   bool allowswide, bool allowssideways)
	: floattype_(type), placement_(placement), ext_(ext), within_(within),
	  style_(style), name_(name), listname_(listName), listcommand_(listCmd),
	  refprefix_(refPrefix), allowedplacement_(allowedplacement), required_(required),
	  usesfloatpkg_(usesfloat), ispredefined_(ispredefined),
	  allowswide_(allowswide), allowssideways_(allowssideways),
	  html_tag_(htmlTag), html_attrib_(htmlAttrib), html_style_(htmlStyle),
	  docbook_caption_(docbookCaption), docbook_tag_(docbookTag),
	  docbook_tag_type_(docbookTagType)
{
	// Implement some edge cases for DocBook. Both docbook_float_type_ and docbook_attr_ must be computed
	// based on the given value of docbookFloatType; docbook_tag_ can still be guessed without correlation.

	// Determine the value of docbook_float_type_.
	{
		// These are the allowed values for docbook_float_type_. Both docbook_attr_ and docbook_tag_type_
		// depend on this list.
		static std::set<std::string> allowedFloatTypes{"figure", "table", "algorithm", "video", "example"};

		// If some type is predetermined in the layout, use it.
		if (!docbookFloatType.empty() && allowedFloatTypes.find(docbookFloatType) != allowedFloatTypes.end())
			docbook_float_type_ = docbookFloatType;
		// Otherwise, try to guess the DocBook type based on the float type.
		else if (floattype_ == "figure" || floattype_ == "graph" || floattype_ == "chart" || floattype_ == "scheme" ||
				floattype_ == "marginfigure") {
			docbook_float_type_ = "figure";
		} else if (floattype_ == "table" || floattype_ == "tableau" || floattype_ == "margintable") {
			docbook_float_type_ = "table";
		} else if (floattype_ == "algorithm") {
			docbook_float_type_ = "algorithm";
		} else if (floattype_ == "video") {
			docbook_float_type_ = "video";
		} else {
			// If nothing matches, return something that will not be valid.
			LYXERR0("Unrecognised float type: " + floattype_);
			docbook_float_type_ = "unknown";
		}
	}

	// Determine the value of docbook_attr_.
	{
		std::set<std::string> achemso = {"chart", "graph", "scheme"};
		bool hasType = docbook_attr_.find("type=") != std::string::npos;

		// For algorithms, a type attribute must be mentioned, if not already present in docbook_attr_.
		if (docbook_float_type_ == "algorithm" && !hasType)
			docbook_attr_ += " type='algorithm'";
		// Specific floats for achemso.
		else if (docbook_float_type_ == "figure" && achemso.find(floattype_) != achemso.end())
			docbook_attr_ += " type='" + floattype_ + "'";

		// Finally, merge in the attributes given in argument.
		if (!docbookAttr.empty())
			docbook_attr_ += " " + docbookAttr;
	}
}


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


std::string Floating::docbookFloatType() const
{
	// All the work is done in the constructor.
	return docbook_float_type_;
}


string Floating::docbookAttr() const
{
	return docbook_attr_;
}


string Floating::docbookTag(bool hasTitle) const
{
	// If there is a preconfigured tag, use it.
	if (!docbook_tag_.empty())
		return docbook_tag_;

	// Otherwise, guess it.
	if (docbookFloatType() == "figure" || docbookFloatType() == "algorithm" || docbookFloatType() == "video") {
		return hasTitle ? "figure" : "informalfigure";
	} else if (docbookFloatType() == "example") {
		return hasTitle ? "example" : "informalexample";
	} else if (docbookFloatType() == "table") {
		return hasTitle ? "table" : "informaltable";
	} else {
		// If nothing matches, return something that will not be valid.
		LYXERR0("Unrecognised float type: " + floattype());
		return "float";
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
	if (!docbook_caption_.empty())
		return docbook_caption_;

	if (docbook_float_type_ == "figure" || docbook_float_type_ == "video" ||
			docbook_float_type_ == "algorithm" || docbook_float_type_ == "example")
		docbook_caption_ = "title";
	else if (floattype_ == "table" || floattype_ == "tableau")
		docbook_caption_ = "caption";
	return docbook_caption_;
}


} // namespace lyx
