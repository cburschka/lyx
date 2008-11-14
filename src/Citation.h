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

namespace lyx {
	
class Buffer;

enum CiteEngine {
	ENGINE_BASIC,
	ENGINE_NATBIB_AUTHORYEAR,
	ENGINE_NATBIB_NUMERICAL,
	ENGINE_JURABIB
};

enum CiteStyle {
	CITE,
	CITET,
	CITEP,
	CITEALT,
	CITEALP,
	CITEAUTHOR,
	CITEYEAR,
	CITEYEARPAR,
	NOCITE
};


class CitationStyle
{
public:
	///
	CitationStyle() : style(CITE), full(false), forceUpperCase(false) {}

	///
	CiteStyle style;
	///
	bool full;
	///
	bool forceUpperCase;
};

} // namespace lyx

#endif
