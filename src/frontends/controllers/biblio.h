// -*- C++ -*-
/**
 * \file biblio.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BIBLIOHELPERS_H
#define BIBLIOHELPERS_H

#include <map>
#include <string>
#include <vector>

class Buffer;

/** Functions of use to citation and bibtex GUI controllers and views */
namespace lyx {
namespace biblio {

class CiteEngine_enum;

CiteEngine_enum getEngine(Buffer const &);


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


/** Each citation engine recognizes only a subset of all possible
 *  citation commands. Given a latex command \c input, this function
 *  returns an appropriate command, valid for \c engine.
 */
std::string const asValidLatexCommand(std::string const & input,
				      CiteEngine_enum const & engine);

/// First entry is the bibliography key, second the data
typedef std::map<std::string, std::string> InfoMap;

/// Returns a vector of bibliography keys
std::vector<std::string> const getKeys(InfoMap const &);

/** Returns the BibTeX data associated with a given key.
    Empty if no info exists. */
std::string const getInfo(InfoMap const &, std::string const &);

// rturn the year from the bibtex data record
std::string const getYear(InfoMap const & map, std::string const & key);

/// return the short form of an authorlist
std::string const getAbbreviatedAuthor(InfoMap const & map, std::string const & key);

// return only the family name
std::string const familyName(std::string const & name);

/** Search a BibTeX info field for the given key and return the
    associated field. */
std::string const parseBibTeX(std::string data, std::string const & findkey);

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
	   std::string const & search_expression,
	   std::vector<std::string>::const_iterator start,
	   Search,
	   Direction,
	   bool caseSensitive=false);


struct CitationStyle {
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
std::vector<CiteStyle> const getCiteStyles(CiteEngine_enum const &);

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
std::vector<std::string> const
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
std::vector<std::string> const
getAuthorYearStrings(std::string const & key,
		     InfoMap const & map,
		     std::vector<CiteStyle> const & styles);

} // namespace biblio
} // namespace lyx

#endif // BIBLIOHELPERS_H
