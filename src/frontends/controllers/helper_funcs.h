// -*- C++ -*-
/**
 * \file helper_funcs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef HELPERFUNCS_H
#define HELPERFUNCS_H

#include <boost/bind.hpp>
#include <utility>
#include <vector>
#include <string>


namespace lyx {
namespace support {
class FileFilterList;
} // namespace support
} // namespace lyx

/** Launch a file dialog and return the chosen file.
    filename: a suggested filename.
    title: the title of the dialog.
    pattern: *.ps etc.
    dir1 = (name, dir), dir2 = (name, dir): extra buttons on the dialog.
*/
std::string const
browseFile(std::string const & filename,
	   std::string const & title,
	   lyx::support::FileFilterList const & filters,
	   bool save = false,
	   std::pair<std::string,std::string> const & dir1 =
	   std::make_pair(std::string(), std::string()),
	   std::pair<std::string,std::string> const & dir2 =
	   std::make_pair(std::string(), std::string()));


/* Wrapper around browseFile which tries to provide a filename
   relative to relpath.  If the relative path is of the form "foo.txt"
   or "bar/foo.txt", then it is returned as relative. OTOH, if it is
   of the form "../baz/foo.txt", an absolute path is returned. This is
   intended to be useful for insets which encapsulate files/
*/
std::string const
browseRelFile(std::string const & filename,
	      std::string const & refpath,
	      std::string const & title,
	      lyx::support::FileFilterList const & filters,
	      bool save = false,
	      std::pair<std::string,std::string> const & dir1 =
	      std::make_pair(std::string(), std::string()),
	      std::pair<std::string,std::string> const & dir2 =
	      std::make_pair(std::string(), std::string()));


/** Launch a file dialog and return the chosen directory.
    pathname: a suggested pathname.
    title: the title of the dialog.
    dir1 = (name, dir), dir2 = (name, dir): extra buttons on the dialog.
*/
std::string const
browseDir(std::string const & pathname,
	   std::string const & title,
	   std::pair<std::string,std::string> const & dir1 =
	   std::make_pair(std::string(), std::string()),
	   std::pair<std::string,std::string> const & dir2 =
	   std::make_pair(std::string(), std::string()));


/// Returns a vector of units that can be used to create a valid LaTeX length.
std::vector<std::string> const getLatexUnits();


/** Functions to extract vectors of the first and second elems from a
    vector<pair<A,B> >
*/
template<class Pair>
std::vector<typename Pair::first_type> const
getFirst(std::vector<Pair> const & pr)
{
	std::vector<typename Pair::first_type> tmp(pr.size());
	std::transform(pr.begin(), pr.end(), tmp.begin(),
		       boost::bind(&Pair::first, _1));
	return tmp;
}

template<class Pair>
std::vector<typename Pair::second_type> const
getSecond(std::vector<Pair> const & pr)
{
	std::vector<typename Pair::second_type> tmp(pr.size());
	std::transform(pr.begin(), pr.end(), tmp.begin(),
		       boost::bind(&Pair::second, _1));
	return tmp;
}

#endif // NOT HELPERFUNCS_H
