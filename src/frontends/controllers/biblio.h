// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file biblio.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef BIBLIOHELPERS_H
#define BIBLIOHELPERS_H

#include <map>

#ifdef __GNUG__
#pragma interface
#endif

/** Functions of use to citation and bibtex GUI controllers and views */
namespace biblio 
{
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

	/** Do the dirty work for the search.
	    Should use through the function above */
	std::vector<string>::const_iterator
	    simpleSearch(InfoMap const & map,
			 std::vector<string> const & keys_to_search,
			 string const & search_expression,
			 std::vector<string>::const_iterator start,
			 Direction,
			 bool caseSensitive=false);

	/// Should use through the function above
	std::vector<string>::const_iterator
	    regexSearch(InfoMap const & map,
			std::vector<string> const & keys_to_search,
			string const & search_expression,
			std::vector<string>::const_iterator start,
			Direction);

} // namespace biblio 

#endif // BIBLIOHELPERS_H
