/**
 * \file TexRow.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Matthias Ettrich
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Cursor.h"
#include "Paragraph.h"
#include "TexRow.h"

#include "mathed/InsetMath.h"

#include "support/debug.h"
#include "support/docstring_list.h"

#include <algorithm>
#include <sstream>


namespace lyx {



bool TexRow::RowEntryList::addEntry(RowEntry const & entry)
{
	if (!entry.is_math) {
		if (text_entry_ < size())
			return false;
		else
			text_entry_ = size();
	}
	forceAddEntry(entry);
	return true;
}


void TexRow::RowEntryList::forceAddEntry(RowEntry const & entry)
{
	if (size() == 0 || !(operator[](size() - 1) == entry))
		push_back(RowEntry(entry));
}


TexRow::TextEntry TexRow::RowEntryList::getTextEntry() const
{
	if (text_entry_ < size())
		return operator[](text_entry_).text;
	return TexRow::text_none;
}


TexRow::RowEntry TexRow::RowEntryList::entry() const
{
	if (0 < size())
		return operator[](0);
	return TexRow::row_none;
}


void TexRow::RowEntryList::append(RowEntryList const & row)
{
	if (text_entry_ >= size())
		text_entry_ = row.text_entry_ + size();
	insert(end(), row.begin(), row.end());
}


TexRow::TextEntry const TexRow::text_none = { -1, 0 };
TexRow::RowEntry const TexRow::row_none = { false, { TexRow::text_none } };


bool TexRow::isNone(TextEntry const & t)
{
	return t.id < 0;
}


bool TexRow::isNone(RowEntry const & r)
{
	return !r.is_math && isNone(r.text);
}


void TexRow::reset(bool enable)
{
	rowlist_.clear();
	current_row_ = RowEntryList();
	enabled_ = enable;
}


TexRow::RowEntry TexRow::textEntry(int id, int pos)
{
	RowEntry entry;
	entry.is_math = false;
	entry.text.pos = pos;
	entry.text.id = id;
	return entry;
}


TexRow::RowEntry TexRow::mathEntry(uid_type id, idx_type cell)
{
	RowEntry entry;
	entry.is_math = true;
	entry.math.cell = cell;
	entry.math.id = id;
	return entry;
}


bool operator==(TexRow::RowEntry const & entry1,
				TexRow::RowEntry const & entry2)
{
	return entry1.is_math == entry2.is_math
		&& (entry1.is_math
			? (entry1.math.id == entry2.math.id
			   && entry1.math.cell == entry2.math.cell)
			: (entry1.text.id == entry2.text.id
			   && entry1.text.pos == entry2.text.pos));
}


bool TexRow::start(RowEntry entry)
{
	if (!enabled_)
		return false;
	return current_row_.addEntry(entry);
}


bool TexRow::start(int id, int pos)
{
	return start(textEntry(id,pos));
}


void TexRow::forceStart(int id, int pos)
{
	if (!enabled_)
		return;
	return current_row_.forceAddEntry(textEntry(id,pos));
}


void TexRow::startMath(uid_type id, idx_type cell)
{
	start(mathEntry(id,cell));
}


void TexRow::newline()
{
	if (!enabled_)
		return;
	rowlist_.push_back(current_row_);
	current_row_ = RowEntryList();
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


void TexRow::append(TexRow const & texrow)
{
	if (!enabled_ || !texrow.enabled_)
		return;
	RowList::const_iterator it = texrow.rowlist_.begin();
	RowList::const_iterator const end = texrow.rowlist_.end();
	if (it == end) {
		current_row_.append(texrow.current_row_);
	} else {
		current_row_.append(*it++);
		rowlist_.push_back(current_row_);
		rowlist_.insert(rowlist_.end(), it, end);
		current_row_ = texrow.current_row_;
	}
}



bool TexRow::getIdFromRow(int row, int & id, int & pos) const
{
	TextEntry t = text_none;
	if (row <= int(rowlist_.size()))
		while (row > 0 && isNone(t = rowlist_[row - 1].getTextEntry()))
			--row;
	id = t.id;
	pos = t.pos;
	return !isNone(t);
}


TexRow::RowEntry TexRow::rowEntryFromCursorSlice(CursorSlice const & slice)
{
	RowEntry entry;
	InsetMath * insetMath = slice.asInsetMath();
	if (insetMath) {
		entry.is_math = 1;
		entry.math.id = insetMath->id();
		entry.math.cell = slice.idx();
	} else if (slice.text()) {
		entry.is_math = 0;
		entry.text.id = slice.paragraph().id();
		entry.text.pos = slice.pos();
	} else {
		// should not happen
		entry = row_none;
	}
	return entry;
}


bool TexRow::sameParOrInsetMath(RowEntry const & entry1,
								RowEntry const & entry2)
{
	return entry1.is_math == entry2.is_math
		&& (entry1.is_math
			? (entry1.math.id == entry2.math.id)
			: (entry1.text.id == entry2.text.id));
}


// assumes it is sameParOrInsetMath
int TexRow::comparePos(RowEntry const & entry1,
					   RowEntry const & entry2)
{
	if (entry1.is_math)
		return entry2.math.cell - entry1.math.cell;
	else
		return entry2.text.pos - entry1.text.pos;
}


// An iterator on RowList that goes top-down, left-right
//
// We assume that the end of RowList does not change, which makes things simpler
//
// Records a pair of iterators on the RowEntryList (row_it_, row_end_) and a
// pair of iterators on the current row (it_, it_end_).
//
// it_ always points to a valid position unless row_it_ == row_end_.
//
// We could turn this into a proper bidirectional iterator, but we don't need as
// much.
//
class TexRow::RowListIterator
{
public:
	RowListIterator(RowList::const_iterator r,
					RowList::const_iterator r_end)
		: row_it_(r), row_end_(r_end),
		  it_(r == r_end ? RowEntryList::const_iterator() : r->begin()),
		  it_end_(r == r_end ? RowEntryList::const_iterator() : r->end())
	{
		normalize();
	}


	RowListIterator() :
		row_it_(RowList::const_iterator()),
		row_end_(RowList::const_iterator()),
		it_(RowEntryList::const_iterator()),
		it_end_(RowEntryList::const_iterator()) { }


	RowEntry const & operator*()
	{
		return *it_;
	}


	RowListIterator & operator++()
	{
		++it_;
		normalize();
		return *this;
	}


	bool atEnd() const
	{
		return row_it_ == row_end_;
	}


	bool operator==(RowListIterator const & a) const
	{
		return row_it_ == a.row_it_ && ((atEnd() && a.atEnd()) || it_ == a.it_);
	}


	bool operator!=(RowListIterator const & a) const { return !operator==(a); }


	// Current row.
	RowList::const_iterator const & row() const
	{
		return row_it_;
	}
private:
	// ensures that it_ points to a valid value unless row_it_ == row_end_
	void normalize()
	{
		if (row_it_ == row_end_)
			return;
		while (it_ == it_end_) {
			++row_it_;
			if (row_it_ != row_end_) {
				it_ = row_it_->begin();
				it_end_ = row_it_->end();
			} else
				return;
		}
	}
	//
	RowList::const_iterator row_it_;
	//
	RowList::const_iterator row_end_;
	//
	RowEntryList::const_iterator it_;
	//
	RowEntryList::const_iterator it_end_;
};


TexRow::RowListIterator TexRow::begin() const
{
	return RowListIterator(rowlist_.begin(), rowlist_.end());
}


TexRow::RowListIterator TexRow::end() const
{
	return RowListIterator(rowlist_.end(), rowlist_.end());
}


std::pair<int,int> TexRow::rowFromDocIterator(DocIterator const & dit) const
{
	bool beg_found = false;
	bool end_is_next = true;
	int end_offset = 1;
	size_t best_slice = 0;
	RowEntry best_entry = row_none;
	size_t const n = dit.depth();
	// this loop finds a pair (best_beg_row,best_end_row) where best_beg_row is
	// the first row of the topmost possible CursorSlice, and best_end_row is
	// the one just before the first row matching the next CursorSlice.
	RowListIterator const begin = this->begin();//necessary disambiguation
	RowListIterator const end = this->end();
	RowListIterator best_beg_entry;
	//best last entry with same pos as the beg_entry, or first entry with pos
	//immediately following the beg_entry
	RowListIterator best_end_entry;
	RowListIterator it = begin;
	for (; it != end; ++it) {
		// Compute the best end row.
		if (beg_found
			&& (!sameParOrInsetMath(*it, *best_end_entry)
				|| comparePos(*it, *best_end_entry) <= 0)
			&& sameParOrInsetMath(*it, best_entry)) {
		    switch (comparePos(*it, best_entry)) {
			case 0:
				// Either it is the last one that matches pos...
				best_end_entry = it;
				end_is_next = false;
				end_offset = 1;
				break;
			case -1: {
				// ...or it is the row preceding the first that matches pos+1
				if (!end_is_next) {
					end_is_next = true;
					if (it.row() != best_end_entry.row())
						end_offset = 0;
					best_end_entry = it;
				}
				break;
			}
			}
		}
		// Compute the best begin row. It is better than the previous one if it
		// matches either at a deeper level, or at the same level but not
		// before.
		for (size_t i = best_slice; i < n; ++i) {
			TexRow::RowEntry entry_i = rowEntryFromCursorSlice(dit[i]);
			if (sameParOrInsetMath(*it, entry_i)) {
				if (comparePos(*it, entry_i) >= 0
					&& (i > best_slice
						|| !beg_found
						|| !sameParOrInsetMath(*it, *best_beg_entry)
						|| (comparePos(*it, *best_beg_entry) <= 0
							&& comparePos(entry_i, *best_beg_entry) != 0)
						)
					) {
					beg_found = true;
					end_is_next = false;
					end_offset = 1;
					best_slice = i;
					best_entry = entry_i;
					best_beg_entry = best_end_entry = it;
				}
				//found CursorSlice
				break;
			}
		}
	}
	if (!beg_found)
		return std::make_pair(-1,-1);
	int const best_beg_row = distance(rowlist_.begin(),
									  best_beg_entry.row()) + 1;
	int const best_end_row = distance(rowlist_.begin(),
									  best_end_entry.row()) + end_offset;
	return std::make_pair(best_beg_row, best_end_row);
}


std::pair<int,int> TexRow::rowFromCursor(Cursor const & cur) const
{
	DocIterator beg = cur.selectionBegin();
	std::pair<int,int> beg_rows = rowFromDocIterator(beg);
	if (cur.selection()) {
		DocIterator end = cur.selectionEnd();
		if (!cur.selIsMultiCell()
			// backwardPos asserts without the following test, IMO it's not my
			// duty to check this.
			&& (end.top().pit() != 0
				|| end.top().idx() != 0
				|| end.top().pos() != 0))
			end.top().backwardPos();
		std::pair<int,int> end_rows = rowFromDocIterator(end);
		return std::make_pair(std::min(beg_rows.first, end_rows.first),
							  std::max(beg_rows.second, end_rows.second));
	} else
		return std::make_pair(beg_rows.first, beg_rows.second);
}


// debugging functions

///
docstring TexRow::asString(RowEntry const & entry)
{
	odocstringstream os;
	if (entry.is_math)
		os << "(1," << entry.math.id << "," << entry.math.cell << ")";
	else
		os << "(0," << entry.text.id << "," << entry.text.pos << ")";
	return os.str();
}


///prepends the texrow to the source given by tex, for debugging purpose
void TexRow::prepend(docstring_list & tex) const
{
	size_type const prefix_length = 25;
	if (tex.size() < rowlist_.size())
		tex.resize(rowlist_.size());
	std::vector<RowEntryList>::const_iterator it = rowlist_.begin();
	std::vector<RowEntryList>::const_iterator const beg = rowlist_.begin();
	std::vector<RowEntryList>::const_iterator const end = rowlist_.end();
	for (; it < end; ++it) {
		docstring entry;
		std::vector<RowEntry>::const_iterator it2 = it->begin();
		std::vector<RowEntry>::const_iterator const end2 = it->end();
		for (; it2 != end2; ++it2)
			entry += asString(*it2);
		if (entry.length() < prefix_length)
			entry = entry + docstring(prefix_length - entry.length(), L' ');
		ptrdiff_t i = it - beg;
		tex[i] = entry + "  " + tex[i];
	}
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
