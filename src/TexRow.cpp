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

#include "support/debug.h"

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
	bool foundid = false;

	// this loop finds the last *nonempty* row with the same id
	// and position <= pos
	RowList::const_iterator bestrow = rowlist.begin();
	RowList::const_iterator it = rowlist.begin();
	RowList::const_iterator const end = rowlist.end();
	for (; it != end; ++it) {
		if (it->id() == id && it->pos() <= pos) {
			foundid = true;
			if (bestrow->id() != id || it->pos() > bestrow->pos())
				bestrow = it;
		} else if (foundid)
			break;
	}
	if (!foundid)
		return rowlist.size();
	return distance(rowlist.begin(), bestrow);
}


} // namespace lyx
