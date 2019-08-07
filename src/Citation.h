// -*- C++ -*-
/**
 * \file Citation.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voß
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CITATION_H
#define CITATION_H

#include "support/docstring.h"
#include <string>
#include <vector>

namespace lyx {

class Buffer;


enum CiteEngineType {
	ENGINE_TYPE_AUTHORYEAR = 1,
	ENGINE_TYPE_NUMERICAL = 2,
	ENGINE_TYPE_DEFAULT = 3,
};


class CitationStyle
{
public:
	///
	CitationStyle() : name("cite"), cmd("cite"), forceUpperCase(false),
		hasStarredVersion(false), hasQualifiedList(false),
		textAfter(false), textBefore(false) {}

	/// the LyX name
	std::string name;
	/// the LaTeX command (might differ from the LyX name)
	std::string cmd;
	/// Optional alternative description what the starred version does (for the GUI)
	std::string stardesc;
	/// Optional tooltip for the starred version
	std::string startooltip;
	/// upper casing author prefixes (van -> Van)
	bool forceUpperCase;
	/// starred version (full author list by default)
	bool hasStarredVersion;
	/// allows for qualified citation lists (a Biblatex feature)
	bool hasQualifiedList;
	/// supports text after the citation
	bool textAfter;
	/// supports text before the citation
	bool textBefore;
};


/**
 * Class for storing information about a given citation item in a given context.
 * This is used in the label and menu string generation process.
 */
class CiteItem
{
public:
	/// The context this citation is displayed
	enum CiteContext{
		Everywhere,
		Dialog,
		Export
	};
	///
	CiteItem() : forceUpperCase(false), Starred(false), isQualified(false),
		context(CiteItem::Everywhere), textAfter(docstring()),
		textBefore(docstring()), max_size(128), max_key_size(128),
		richtext(false) {}
	/// requests upper casing author prefixes (van -> Van)
	bool forceUpperCase;
	/// is starred version (full author list by default)
	bool Starred;
	/// is a real qualified list
	bool isQualified;
	/// where this to be displayed?
	CiteItem::CiteContext context;
	/// text after the citation
	docstring textAfter;
	/// text before the citation
	docstring textBefore;
	///
	typedef std::vector<std::pair<docstring, docstring>> QualifiedList;
	/// Qualified lists's pre texts
	QualifiedList pretexts;
	///
	QualifiedList getPretexts() const { return pretexts; }
	/// Qualified lists's post texts
	QualifiedList posttexts;
	///
	QualifiedList getPosttexts() const { return posttexts; }
	/// the maximum display size as a label
	size_t max_size;
	/// the maximum size of the processed keys
	/// (limited for performance reasons)
	size_t max_key_size;
	/// output richtext information?
	bool richtext;
};

} // namespace lyx

#endif
