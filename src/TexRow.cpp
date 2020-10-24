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

#include "TexRow.h"

#include "Buffer.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "Paragraph.h"

#include "mathed/InsetMath.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstring_list.h"
#include "support/lassert.h"

#include <algorithm>
#include <iterator>
#include <sstream>

using namespace std;


namespace lyx {


TexString::TexString(docstring s)
	: str(move(s)), texrow(TexRow())
{
	texrow.setRows(1 + count(str.begin(), str.end(), '\n'));
}


TexString::TexString(docstring s, TexRow t)
	: str(move(s)), texrow(move(t))
{
	validate();
}


void TexString::validate()
{
	size_t lines = 1 + count(str.begin(), str.end(), '\n');
	size_t rows = texrow.rows();
	bool valid = lines == rows;
	if (!valid)
		LYXERR0("TexString has " << lines << " lines but " << rows << " rows." );
	// Assert in devel mode.  This is important to catch bugs early, otherwise
	// they might be hard to notice and find.  Recover gracefully in release
	// mode.
	LASSERT(valid, texrow.setRows(lines));
}


bool TexRow::RowEntryList::addEntry(RowEntry entry)
{
	switch (entry.type) {
	case text_entry:
		if (isNone(text_entry_))
			text_entry_ = entry.text;
		else if (!v_.empty() && TexRow::sameParOrInsetMath(v_.back(), entry))
			return false;
		break;
	default:
		break;
	}
	forceAddEntry(entry);
	return true;
}


void TexRow::RowEntryList::forceAddEntry(RowEntry entry)
{
	if (v_.empty() || !(v_.back() == entry))
		v_.push_back(entry);
}


TexRow::TextEntry TexRow::RowEntryList::getTextEntry() const
{
	if (!isNone(text_entry_))
		return text_entry_;
	return TexRow::text_none;
}


void TexRow::RowEntryList::append(RowEntryList row)
{
	if (isNone(text_entry_))
		text_entry_ = row.text_entry_;
	move(row.begin(), row.end(), back_inserter(v_));
}


TexRow::TexRow()
{
	reset();
}


TexRow::TextEntry const TexRow::text_none = { -1, 0 };
TexRow::RowEntry const TexRow::row_none = TexRow::textEntry(-1, 0);


//static
bool TexRow::isNone(TextEntry t)
{
	return t.id < 0;
}


//static
bool TexRow::isNone(RowEntry r)
{
	return r.type == text_entry && isNone(r.text);
}


void TexRow::reset()
{
	rowlist_.clear();
	newline();
}


TexRow::RowEntryList & TexRow::currentRow()
{
	return rowlist_.back();
}


//static
TexRow::RowEntry TexRow::textEntry(int id, pos_type pos)
{
	RowEntry entry;
	entry.type = text_entry;
	entry.text.pos = pos;
	entry.text.id = id;
	return entry;
}


//static
TexRow::RowEntry TexRow::mathEntry(uid_type id, idx_type cell)
{
	RowEntry entry;
	entry.type = math_entry;
	entry.math.cell = cell;
	entry.math.id = id;
	return entry;
}


//static
TexRow::RowEntry TexRow::beginDocument()
{
	RowEntry entry;
	entry.type = begin_document;
	entry.begindocument = {};
	return entry;
}


bool operator==(TexRow::RowEntry entry1, TexRow::RowEntry entry2)
{
	if (entry1.type != entry2.type)
		return false;
	switch (entry1.type) {
	case TexRow::text_entry:
		return entry1.text.id == entry2.text.id
			&& entry1.text.pos == entry2.text.pos;
	case TexRow::math_entry:
		return entry1.math.id == entry2.math.id
			&& entry1.math.cell == entry2.math.cell;
	case TexRow::begin_document:
		return true;
	default:
		return false;
	}
}


bool TexRow::start(RowEntry entry)
{
	return currentRow().addEntry(entry);
}


bool TexRow::start(int id, pos_type pos)
{
	return start(textEntry(id,pos));
}


void TexRow::forceStart(int id, pos_type pos)
{
	return currentRow().forceAddEntry(textEntry(id,pos));
}


void TexRow::startMath(uid_type id, idx_type cell)
{
	start(mathEntry(id,cell));
}


void TexRow::newline()
{
	rowlist_.push_back(RowEntryList());
}


void TexRow::newlines(size_t num_lines)
{
	while (num_lines--)
		newline();
}


void TexRow::append(TexRow other)
{
	RowList::iterator it = other.rowlist_.begin();
	RowList::iterator const end = other.rowlist_.end();
	LASSERT(it != end, return);
	currentRow().append(move(*it++));
	move(it, end, back_inserter(rowlist_));
}


pair<TexRow::TextEntry, TexRow::TextEntry>
TexRow::getEntriesFromRow(int const row) const
{
	// FIXME: Take math entries into account, take table cells into account and
	//        get rid of the ad hoc special text entry for each row.
	//
	// FIXME: A yellow note alone on its paragraph makes the reverse-search on
	//        the subsequent line inaccurate. Get rid of text entries that
	//        correspond to no output by delaying their addition, at the level
	//        of otexrowstream, until a character is actually output.
	//
	LYXERR(Debug::LATEX, "getEntriesFromRow: row " << row << " requested");

	// check bounds for row - 1, our target index
	if (row <= 0)
		return {text_none, text_none};
	size_t const i = static_cast<size_t>(row - 1);
	if (i >= rowlist_.size())
		return {text_none, text_none};

	// find the start entry
	TextEntry const start = [&]() {
		for (size_t j = i; j > 0; --j) {
			if (!isNone(rowlist_[j].getTextEntry()))
				return rowlist_[j].getTextEntry();
			// Check the absence of begin_document at row j. The begin_document row
			// entry is used to prevent mixing of body and preamble.
			for (RowEntry entry : rowlist_[j])
				if (entry.type == begin_document)
					return text_none;
		}
		return text_none;
	} ();

	// find the end entry
	TextEntry end = [&]() {
		if (isNone(start))
			return text_none;
		// select up to the last position of the starting paragraph as a
		// fallback
		TextEntry last_pos = {start.id, -1};
		// find the next occurence of paragraph start.id
		for (size_t j = i + 1; j < rowlist_.size(); ++j) {
			for (RowEntry entry : rowlist_[j]) {
				if (entry.type == begin_document)
					// what happens in the preamble remains in the preamble
					return last_pos;
				if (entry.type == text_entry && entry.text.id == start.id)
					return entry.text;
			}
		}
		return last_pos;
	} ();

	// The following occurs for a displayed math inset for instance (for good
	// reasons involving subtleties of the algorithm in getRowFromDocIterator).
	// We want this inset selected.
	if (start.id == end.id && start.pos == end.pos)
		++end.pos;

	return {start, end};
}


pair<DocIterator, DocIterator> TexRow::getDocIteratorsFromRow(
    int const row,
    Buffer const & buf) const
{
	TextEntry start, end;
	tie(start,end) = getEntriesFromRow(row);
	return getDocIteratorsFromEntries(start, end, buf);
}


//static
pair<DocIterator, DocIterator> TexRow::getDocIteratorsFromEntries(
	    TextEntry start,
	    TextEntry end,
	    Buffer const & buf)
{
	auto set_pos = [](DocIterator & dit, pos_type pos) {
		dit.pos() = (pos >= 0) ? min(pos, dit.lastpos())
		                       // negative pos values are counted from the end
		                       : max(dit.lastpos() + pos + 1, pos_type(0));
	};
	// Finding start
	DocIterator dit_start = buf.getParFromID(start.id);
	if (dit_start)
		set_pos(dit_start, start.pos);
	// Finding end
	DocIterator dit_end = buf.getParFromID(end.id);
	if (dit_end) {
		set_pos(dit_end, end.pos);
		// Step backwards to prevent selecting the beginning of another
		// paragraph.
		if (dit_end.pos() == 0 && !dit_end.top().at_cell_begin()) {
			CursorSlice end_top = dit_end.top();
			end_top.backwardPos();
			if (dit_start && end_top != dit_start.top())
				dit_end.top() = end_top;
		}
		dit_end.boundary(true);
	}
	return {dit_start, dit_end};
}


//static
FuncRequest TexRow::goToFunc(TextEntry start, TextEntry end)
{
	return {LFUN_PARAGRAPH_GOTO,
			convert<string>(start.id) + " " + convert<string>(start.pos) + " " +
			convert<string>(end.id) + " " + convert<string>(end.pos)};
}


FuncRequest TexRow::goToFuncFromRow(int const row) const
{
	TextEntry start, end;
	tie(start,end) = getEntriesFromRow(row);
	LYXERR(Debug::LATEX,
	       "goToFuncFromRow: for row " << row << ", TexRow has found "
	       "start (id=" << start.id << ",pos=" << start.pos << "), "
	       "end (id=" << end.id << ",pos=" << end.pos << ")");
	return goToFunc(start, end);
}


//static
TexRow::RowEntry TexRow::rowEntryFromCursorSlice(CursorSlice const & slice)
{
	RowEntry entry;
	InsetMath * insetMath = slice.asInsetMath();
	if (insetMath) {
		entry.type = math_entry;
		entry.math.id = insetMath->id();
		entry.math.cell = slice.idx();
	} else if (slice.text()) {
		entry.type = text_entry;
		entry.text.id = slice.paragraph().id();
		entry.text.pos = slice.pos();
	} else
		LASSERT(false, return row_none);
	return entry;
}


//static
bool TexRow::sameParOrInsetMath(RowEntry entry1, RowEntry entry2)
{
	if (entry1.type != entry2.type)
		return false;
	switch (entry1.type) {
	case TexRow::text_entry:
		return entry1.text.id == entry2.text.id;
	case TexRow::math_entry:
		return entry1.math.id == entry2.math.id;
	case TexRow::begin_document:
		return true;
	default:
		return false;
	}
}


//static
int TexRow::comparePos(RowEntry entry1, RowEntry entry2)
{
	// assume it is sameParOrInsetMath
	switch (entry1.type /* equal to entry2.type */) {
	case TexRow::text_entry:
		return entry2.text.pos - entry1.text.pos;
	case TexRow::math_entry:
		return entry2.math.cell - entry1.math.cell;
	case TexRow::begin_document:
		return 0;
	default:
		return 0;
	}
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


pair<int,int> TexRow::rowFromDocIterator(DocIterator const & dit) const
{
	// Do not change anything in this algorithm if unsure.
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
			RowEntry entry_i = rowEntryFromCursorSlice(dit[i]);
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
		return make_pair(-1,-1);
	int const best_beg_row = distance(rowlist_.begin(),
									  best_beg_entry.row()) + 1;
	int const best_end_row = distance(rowlist_.begin(),
									  best_end_entry.row()) + end_offset;
	return make_pair(best_beg_row, best_end_row);
}


pair<int,int> TexRow::rowFromCursor(Cursor const & cur) const
{
	DocIterator beg = cur.selectionBegin();
	pair<int,int> beg_rows = rowFromDocIterator(beg);
	if (cur.selection()) {
		DocIterator end = cur.selectionEnd();
		if (!cur.selIsMultiCell() && !end.top().at_cell_begin())
			end.top().backwardPos();
		pair<int,int> end_rows = rowFromDocIterator(end);
		return make_pair(min(beg_rows.first, end_rows.first),
		                 max(beg_rows.second, end_rows.second));
	} else
		return make_pair(beg_rows.first, beg_rows.second);
}


size_t TexRow::rows() const
{
	return rowlist_.size();
}


void TexRow::setRows(size_t r)
{
	rowlist_.resize(r, RowEntryList());
}


// debugging functions

///
docstring TexRow::asString(RowEntry entry)
{
	odocstringstream os;
	switch (entry.type) {
	case TexRow::text_entry:
		os << "(par " << entry.text.id << "," << entry.text.pos << ")";
		break;
	case TexRow::math_entry:
		os << "(" << entry.math.id << "," << entry.math.cell << ")";
		break;
	case TexRow::begin_document:
		os << "(begin_document)";
		break;
	default:
		break;
	}
	return os.str();
}


///prepends the texrow to the source given by tex, for debugging purpose
void TexRow::prepend(docstring_list & tex) const
{
	size_type const prefix_length = 25;
	if (tex.size() < rowlist_.size())
		tex.resize(rowlist_.size());
	auto it = rowlist_.cbegin();
	auto const beg = rowlist_.cbegin();
	auto const end = rowlist_.cend();
	for (; it < end; ++it) {
		docstring entry;
		for (RowEntry const & e : *it)
			entry += asString(e);
		if (entry.length() < prefix_length)
			entry = entry + docstring(prefix_length - entry.length(), ' ');
		ptrdiff_t i = it - beg;
		tex[i] = entry + "  " + tex[i];
	}
}


} // namespace lyx
