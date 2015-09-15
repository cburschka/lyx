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

#include "DocIterator.h"
#include "Paragraph.h"
#include "TexRow.h"

#include "support/debug.h"

#include <algorithm>


namespace lyx {


void TexRow::reset(bool enable)
{
	rowlist.clear();
	lastid = -1;
	lastpos = -1;
	enabled_ = enable;
}


void TexRow::start(int id, int pos)
{
	if (!enabled_ || started)
		return;
	lastid = id;
	lastpos = pos;
	started = true;
}


void TexRow::newline()
{
	if (!enabled_)
		return;
	RowList::value_type tmp(lastid, lastpos);
	rowlist.push_back(tmp);
	started = false;
}

void TexRow::newlines(int num_lines)
{
	if (!enabled_)
		return;
	for (int i = 0; i < num_lines; ++i) {
		newline();
	}
}

void TexRow::finalize()
{
	if (!enabled_)
		return;
	newline();
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


std::pair<int,int> TexRow::rowFromDocIterator(DocIterator const & dit) const
{
	bool found = false;
	size_t best_slice = 0;
	size_t const n = dit.depth();
	// this loop finds the last row of the topmost possible CursorSlice
	RowList::const_iterator best_beg_row = rowlist.begin();
	RowList::const_iterator best_end_row = rowlist.begin();
	RowList::const_iterator it = rowlist.begin();
	RowList::const_iterator const end = rowlist.end();
	for (; it != end; ++it) {
		if (found) {
			// Compute the best end row. It is the one that matches pos+1.
			CursorSlice const & best = dit[best_slice];
			if (best.text()
				&& it->id() == best.paragraph().id()
				&& it->pos() == best.pos() + 1
				&& (best_end_row->id() != it->id()
					|| best_end_row->pos() < it->pos()))
					best_end_row = it;
		}
		for (size_t i = best_slice; i < n && dit[i].text(); ++i) {
			int const id = dit[i].paragraph().id();
			if (it->id() == id) {
				if (it->pos() <= dit[i].pos()
					&& (best_beg_row->id() != id
						|| it->pos() > best_beg_row->pos())) {
					found = true;
					best_slice = i;
					best_beg_row = best_end_row = it;
				}
				//found CursorSlice
				break;
			}
		}
	}
	if (!found)
		return std::make_pair(-1,-1);
	int const beg_i = distance(rowlist.begin(), best_beg_row) + 1;
	// remove one to the end
	int const end_i = std::max(beg_i,
							   (int)distance(rowlist.begin(), best_end_row));
	return std::make_pair(beg_i,end_i);
}


LyXErr & operator<<(LyXErr & l, TexRow & texrow)
{
	if (l.enabled()) {
		for (int i = 0; i < texrow.rows(); i++) {
			int id,pos;
			if (texrow.getIdFromRow(i+1,id,pos) && id>0)
				l << i+1 << ":" << id << ":" << pos << "\n";
		}
	}
	return l;
}



} // namespace lyx
