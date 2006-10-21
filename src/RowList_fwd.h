// -*- C++ -*-
/**
 * \file RowList_fwd.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef ROW_LIST_FWD_H
#define ROW_LIST_FWD_H

#include "lyxrow.h"

#include <vector>
#include <map>


namespace lyx {

/**
 * Each paragraph is broken up into a number of rows on the screen.
 * This is a list of such on-screen rows, ordered from the top row
 * downwards.
 */
typedef std::vector<Row> RowList;
///
typedef std::map<size_type, size_type> RowSignature;


} // namespace lyx

#endif
