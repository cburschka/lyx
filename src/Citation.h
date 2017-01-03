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

#include <string>

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
		hasStarredVersion(false), textAfter(false), textBefore(false) {}

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
	/// supports text after the citation
	bool textAfter;
	/// supports text before the citation
	bool textBefore;
};

} // namespace lyx

#endif
