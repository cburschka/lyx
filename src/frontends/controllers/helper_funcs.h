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

#ifndef HELPERFUNCS_H
#define HELPERFUNCS_H

#include <utility> // pair

#ifdef __GNUG__
#pragma interface
#endif

/** Functions to convert a string to/from a vector. */

///
string const
getStringFromVector(std::vector<string> const & vec, string const & delim=",");

///
std::vector<string> const
getVectorFromString(string const & str, string const & delim=",");

class LyXView;
 
/** Launch a file dialog and return the chosen file.
    filename: a suggested filename.
    title: the title of the dialog.
    pattern: *.ps etc.
    dir1 = (name, dir), dir2 = (name, dir): extra buttons on the dialog.
*/
string const browseFile(LyXView *lv, string const & filename,
			string const & title,
			string const & pattern, 
			std::pair<string,string> const & dir1,
			std::pair<string,string> const & dir2);

/** Functions to extract vectors of the first and second elems from a
    vector<pair<A,B> >
*/

template<class Pair>
struct firster {
	typedef typename Pair::first_type first_type;
	first_type const & operator()(Pair const & p) { return p.first; }
};

template<class Pair>
struct seconder {
	typedef typename Pair::second_type second_type;
	second_type const & operator()(Pair const & p) { return p.second; }
};

///
template<class Pair>
std::vector<typename Pair::first_type> const
getFirst(std::vector<Pair> const & pr)
{
	std::vector<typename Pair::first_type> tmp(pr.size());
	std::transform(pr.begin(), pr.end(), tmp.begin(), firster<Pair>());
	return tmp;
}

///
template<class Pair>
std::vector<typename Pair::second_type> const
getSecond(std::vector<Pair> const & pr)
{
	std::vector<typename Pair::second_type> tmp(pr.size());
	std::transform(pr.begin(), pr.end(), tmp.begin(), seconder<Pair>());
	return tmp;
}


#endif // HELPERFUNCS_H
