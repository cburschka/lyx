// -*- C++ -*-
/**
 * \file ghelpers.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GHELPERS_H
#define GHELPERS_H

#include <string>
#include <vector>

namespace lyx {
namespace frontend {

/** name is the name of the glade file, without path or extension.
 *  Eg, "aboutlyx", "tableCreate".
 */
std::string const findGladeFile(std::string const & name);

template<class A>
typename std::vector<A>::size_type
findPos(std::vector<A> const & vec, A const & val)
{
	typename std::vector<A>::const_iterator it =
		std::find(vec.begin(), vec.end(), val);
	if (it == vec.end())
		return 0;
	return std::distance(vec.begin(), it);
}

} // namespace frontend
} // namespace lyx

#endif // NOT GHELPERS_H
