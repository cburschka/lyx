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
 * \file ControlCitation.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef CONTROLCITATION_H
#define CONTROLCITATION_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlCommand.h"

/** A controller for Citation dialogs. All citation-specific functionality
    should go in here.
 */
class ControlCitation : public ControlCommand
{
public:
	///
	typedef std::map<string, string> InfoMap;
	///
	typedef std::map<string, string>::value_type InfoMapValue;
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
	///
	ControlCitation(LyXView &, Dialogs &);
	/// A vector of bibliography keys
	std::vector<string> const getBibkeys();
	/// Returns a reference to the map of stored keys
	InfoMap const & bibkeysInfo() const { return bibkeysInfo_; }
	/** Returns the BibTeX data associated with a given key.
	    Empty if no info exists. */
	string const getBibkeyInfo(string const &);
private:
	/// Clean up, then hide dialog.
	virtual void clearParams();
	/// The info associated with each key
	InfoMap bibkeysInfo_;
};

/** Helper functions, of possible use to all frontends
 */

/** Search a BibTeX info field for the given key and return the
    associated field. */
string const parseBibTeX(string data, string const & findkey);

/** Returns an iterator to the first key that meets the search criterion,
    or end() if unsuccessful.

    User supplies :
    the controller with the map of bibkeys info,
    the vector of keys to be searched,
    the search criterion,
    an iterator defining the starting point of the search,
    an enum defining a Simple or Regex search,
    an enum defining the search direction.
*/

std::vector<string>::const_iterator
searchKeys(ControlCitation const & controller,
	   std::vector<string> const & keys_to_search,
	   string const & search_expression,
	   std::vector<string>::const_iterator start,
	   ControlCitation::Search,
	   ControlCitation::Direction,
	   bool caseSensitive=false);

/// Do the dirty work for the search. Should use through the function above
std::vector<string>::const_iterator
simpleSearch(ControlCitation const & controller,
	     std::vector<string> const & keys_to_search,
	     string const & search_expression,
	     std::vector<string>::const_iterator start,
	     ControlCitation::Direction,
	     bool caseSensitive=false);

/// Should use through the function above
std::vector<string>::const_iterator
regexSearch(ControlCitation const & controller,
	    std::vector<string> const & keys_to_search,
	    string const & search_expression,
	    std::vector<string>::const_iterator start,
	    ControlCitation::Direction);
#endif // CONTROLCITATION_H



