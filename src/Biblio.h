// -*- C++ -*-
/**
 * \file Biblio.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BIBLIO_H
#define BIBLIO_H

#include "Buffer.h"
#include "support/docstring.h"

#include <vector>

namespace lyx {
	
namespace biblio {
	
	extern const docstring TheBibliographyRef;

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
		CITEYEARPAR
	};


	enum Search {
		SIMPLE,
		REGEX
	};


	enum Direction {
		FORWARD,
		BACKWARD
	};


/** Fills keys with BibTeX information derived from the various
 *  in this document or its master document.
 */
	void fillWithBibKeys(Buffer const * const buf, 
		std::vector<std::pair<std::string, docstring> > & keys);

/** Each citation engine recognizes only a subset of all possible
	*  citation commands. Given a latex command \c input, this function
	*  returns an appropriate command, valid for \c engine.
 */
	std::string const asValidLatexCommand(std::string const & input,
	                                      CiteEngine const engine);

/// First entry is the bibliography key, second the data
	typedef std::map<std::string, docstring> InfoMap;

/// Returns a vector of bibliography keys
	std::vector<std::string> const getKeys(InfoMap const &);

/** Returns the BibTeX data associated with a given key.
	Empty if no info exists. */
	docstring const getInfo(InfoMap const &, std::string const & key);

/// return the year from the bibtex data record
	docstring const getYear(InfoMap const & map, std::string const & key);

/// return the short form of an authorlist
	docstring const getAbbreviatedAuthor(InfoMap const & map, std::string const & key);

// return only the family name
	docstring const familyName(docstring const & name);

/** Search a BibTeX info field for the given key and return the
	associated field. */
	docstring const parseBibTeX(docstring data, std::string const & findkey);

/** Returns an iterator to the first key that meets the search
	criterion, or end() if unsuccessful.

    User supplies :
	the InfoMap of bibkeys info,
	the vector of keys to be searched,
	the search criterion,
	an iterator defining the starting point of the search,
	an enum defining a Simple or Regex search,
	an enum defining the search direction.
 */

	std::vector<std::string>::const_iterator
			searchKeys(InfoMap const & map,
			           std::vector<std::string> const & keys_to_search,
			           docstring const & search_expression,
			           std::vector<std::string>::const_iterator start,
	 Search,
	Direction,
 bool caseSensitive=false);


	class CitationStyle {
		public:
			///
			CitationStyle(CiteStyle s = CITE, bool f = false, bool force = false)
			: style(s), full(f), forceUCase(force) {}
			/// \param latex_str a LaTeX command, "cite", "Citep*", etc
			CitationStyle(std::string const & latex_str);
			///
			std::string const asLatexStr() const;
			///
			CiteStyle style;
			///
			bool full;
			///
			bool forceUCase;
	};


/// Returns a vector of available Citation styles.
	std::vector<CiteStyle> const getCiteStyles(CiteEngine const );

/**
	"Translates" the available Citation Styles into strings for this key.
	The returned string is displayed by the GUI.


	[XX] is used in place of the actual reference
	Eg, the vector will contain: [XX], Jones et al. [XX], ...

   User supplies :
	the key,
	the InfoMap of bibkeys info,
	the available citation styles
 */
	std::vector<docstring> const
			getNumericalStrings(std::string const & key,
			                    InfoMap const & map,
			                    std::vector<CiteStyle> const & styles);

/**
	"Translates" the available Citation Styles into strings for this key.
	The returned string is displayed by the GUI.

   Eg, the vector will contain:
	Jones et al. (1990), (Jones et al. 1990), Jones et al. 1990, ...

   User supplies :
	the key,
	the InfoMap of bibkeys info,
	the available citation styles
 */
	std::vector<docstring> const
			getAuthorYearStrings(std::string const & key,
			                     InfoMap const & map,
			                     std::vector<CiteStyle> const & styles);

} // namespace biblio
} // namespace lyx
#endif
