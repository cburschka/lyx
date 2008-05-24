// -*- C++ -*-
/**
 * \file docstring_list.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DOCSTRINGLIST_H
#define DOCSTRINGLIST_H

#include "support/docstring.h"

#include <vector>

namespace lyx {

/**
 * Class for storing docstring list.
 * std::vector can not be forward declared in a simple way. Creating a class solves
 * this problem.
 */
class docstring_list : public std::vector<docstring>
{
public:
	docstring_list(): std::vector<docstring>() {}

	docstring_list(std::vector<docstring> const & v) : std::vector<docstring>(v)
	{}
};

} // namespace lyx

#endif // DOCSTRINGLIST_H
