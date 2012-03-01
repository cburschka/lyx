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
};


class CitationStyle
{
public:
	///
	CitationStyle() : cmd("cite"), forceUpperCase(false), fullAuthorList(false),
		textAfter(false), textBefore(false) {}

	/// the LaTeX command
	std::string cmd;
	/// upper casing author prefixes (van -> Van)
	bool forceUpperCase;
	/// expanding the full author list
	bool fullAuthorList;
	/// supports text after the citation
	bool textAfter;
	/// supports text before the citation
	bool textBefore;
};

} // namespace lyx

#endif
