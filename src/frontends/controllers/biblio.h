// -*- C++ -*-
/**
 * \file biblio.h
 * See the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef BIBLIOHELPERS_H
#define BIBLIOHELPERS_H

#include <map>
#include <vector>

#ifdef __GNUG__
#pragma interface
#endif

/** Functions of use to citation and bibtex GUI controllers and views */
namespace biblio
{
	///
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
	///
	enum Search {
		///
		SIMPLE,
		///
		REGEX
	};
	///
	enum Direction {
		///
		FORWARD,
		///
		BACKWARD
	};

	/// First entry is the bibliography key, second the data
	typedef std::map<string, string> InfoMap;

	/// Returns a vector of bibliography keys
	std::vector<string> const getKeys(InfoMap const &);

	/** Returns the BibTeX data associated with a given key.
	    Empty if no info exists. */
	string const getInfo(InfoMap const &, string const &);

	// rturn the year from the bibtex data record
	string const getYear(InfoMap const & map, string const & key);

	/// return the short form of an authorlist
	string const getAbbreviatedAuthor(InfoMap const & map, string const & key);

	// return only the family name
	string const familyName(string const & name);

	/** Search a BibTeX info field for the given key and return the
	    associated field. */
	string const parseBibTeX(string data, string const & findkey);

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

	std::vector<string>::const_iterator
		searchKeys(InfoMap const & map,
			   std::vector<string> const & keys_to_search,
			   string const & search_expression,
			   std::vector<string>::const_iterator start,
			   Search,
			   Direction,
			   bool caseSensitive=false);

	/// Type returned by getCitationStyle, below
	struct CitationStyle {
		///
		CitationStyle() : style(CITE), full(false), forceUCase(false) {}
		///
		CiteStyle style;
		///
		bool full;
		///
		bool forceUCase;
	};
	/// Given the LaTeX command, return the appropriate CitationStyle
	CitationStyle const getCitationStyle(string const & command);

	/** Returns the LaTeX citation command

	    User supplies :
	    The CiteStyle enum,
	    a flag forcing the full author list,
	    a flag forcing upper case, e.g. "della Casa" becomes "Della Case"
	 */
	string const getCiteCommand(CiteStyle, bool full, bool forceUCase);

	/// Returns a vector of available Citation styles.
	std::vector<CiteStyle> const getCiteStyles(bool usingNatbib);

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
	std::vector<string> const
		getNumericalStrings(string const & key,
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
	std::vector<string> const
		getAuthorYearStrings(string const & key,
				     InfoMap const & map,
				     std::vector<CiteStyle> const & styles);
} // namespace biblio

#endif // BIBLIOHELPERS_H
