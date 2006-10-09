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

#include "support/docstring.h"

#include <boost/bind.hpp>
#include <utility>
#include <vector>
#include <string>
#include <algorithm>


namespace lyx {

namespace support {
class FileFilterList;
} // namespace support


namespace frontend {

/** Launch a file dialog and return the chosen file.
    filename: a suggested filename.
    title: the title of the dialog.
    pattern: *.ps etc.
    dir1 = (name, dir), dir2 = (name, dir): extra buttons on the dialog.
*/
lyx::docstring const
browseFile(lyx::docstring const & filename,
	   lyx::docstring const & title,
	   support::FileFilterList const & filters,
	   bool save = false,
	   std::pair<lyx::docstring,lyx::docstring> const & dir1 =
	   std::make_pair(lyx::docstring(), lyx::docstring()),
	   std::pair<lyx::docstring,lyx::docstring> const & dir2 =
	   std::make_pair(lyx::docstring(), lyx::docstring()));


/** Wrapper around browseFile which tries to provide a filename
    relative to relpath.  If the relative path is of the form "foo.txt"
    or "bar/foo.txt", then it is returned as relative. OTOH, if it is
    of the form "../baz/foo.txt", an absolute path is returned. This is
    intended to be useful for insets which encapsulate files/
*/
lyx::docstring const
browseRelFile(lyx::docstring const & filename,
	      lyx::docstring const & refpath,
	      lyx::docstring const & title,
	      support::FileFilterList const & filters,
	      bool save = false,
	      std::pair<lyx::docstring,lyx::docstring> const & dir1 =
	      std::make_pair(lyx::docstring(), lyx::docstring()),
	      std::pair<lyx::docstring,lyx::docstring> const & dir2 =
	      std::make_pair(lyx::docstring(), lyx::docstring()));


/** Wrapper around browseFile which tries to provide a filename
 *  relative to the user or system directory. The dir, name and ext
 *  parameters have the same meaning as in the
 *  lyx::support::LibFileSearch function.
 */
lyx::docstring const
browseLibFile(lyx::docstring const & dir,
	      lyx::docstring const & name,
	      lyx::docstring const & ext,
	      lyx::docstring const & title,
	      support::FileFilterList const & filters);


/** Launch a file dialog and return the chosen directory.
    pathname: a suggested pathname.
    title: the title of the dialog.
    dir1 = (name, dir), dir2 = (name, dir): extra buttons on the dialog.
*/
lyx::docstring const
browseDir(lyx::docstring const & pathname,
	   lyx::docstring const & title,
	   std::pair<lyx::docstring,lyx::docstring> const & dir1 =
	   std::make_pair(lyx::docstring(), lyx::docstring()),
	   std::pair<lyx::docstring,lyx::docstring> const & dir2 =
	   std::make_pair(lyx::docstring(), lyx::docstring()));


/// Returns a vector of units that can be used to create a valid LaTeX length.
std::vector<lyx::docstring> const getLatexUnits();


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

} // namespace frontend
} // namespace lyx

#endif // NOT HELPERFUNCS_H
