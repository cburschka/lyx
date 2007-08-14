/**
 * \file TexRow.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Matthias Ettrich
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "TexRow.h"
#include "debug.h"

#include <algorithm>


namespace lyx {


void TexRow::reset()
{
	rowlist.clear();
	lastid = -1;
	lastpos = -1;
}


void TexRow::start(int id, int pos)
{
	lastid = id;
	lastpos = pos;
}


void TexRow::newline()
{
	int const id = lastid;
	RowList::value_type tmp(id, lastpos);
	rowlist.push_back(tmp);
}


bool TexRow::getIdFromRow(int row, int & id, int & pos) const
{
	if (row <= 0 || row > int(rowlist.size())) {
		id = -1;
		pos = 0;
		return false;
	}

	id = rowlist[row - 1].id();
	pos = rowlist[row - 1].pos();
	return true;
}


int TexRow::getRowFromIdPos(int id, int pos) const
{
	int bestrow = 0;
	bool foundid = false;

	// this loop finds the last *nonempty* row whith the same id
	// and position <= pos
	for (unsigned r = 0, n = rowlist.size(); r != n; ++r) {
		if (rowlist[r].id() == id && rowlist[r].pos() <= pos) {
			foundid = true;
			if (rowlist[bestrow].id() != id || rowlist[r].pos() > rowlist[bestrow].pos())
				bestrow = r;
		} else if (foundid)
			break;
	}
	if (!foundid)
		return rowlist.size();
	return bestrow;
}


} // namespace lyx
