/**
 * \file changes.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Michael Gerz
 *
 * Full author contact details are available in file CREDITS.
 *
 * Record changes in a paragraph.
 */

#include <config.h>

#include "changes.h"
#include "debug.h"

#include <boost/assert.hpp>


namespace lyx {

using std::endl;
using std::string;


bool operator==(Change const & l, Change const & r)
{
	return l.type == r.type && l.author == r.author
		&& l.changetime == r.changetime;
}


bool operator!=(Change const & l, Change const & r)
{
	return !(l == r);
}


bool operator==(Changes::Range const & r1, Changes::Range const & r2)
{
	return r1.start == r2.start && r1.end == r2.end;
}


bool operator!=(Changes::Range const & r1, Changes::Range const & r2)
{
	return !(r1 == r2);
}


bool Changes::Range::contains(Range const & r) const
{
	return r.start >= start && r.end <= end;
}


bool Changes::Range::contains(pos_type const pos) const
{
	return pos >= start && pos < end;
}


bool Changes::Range::intersects(Range const & r) const
{
	return r.start < end && r.end > start; // end itself is not in the range!
}


void Changes::set(Change const & change, pos_type const pos)
{
	set(change, pos, pos + 1);
}


void Changes::set(Change const & change, pos_type const start, pos_type const end)
{
	if (lyxerr.debugging(Debug::CHANGES)) {
		lyxerr[Debug::CHANGES] << "setting change (type: " << change.type
			<< ", author: " << change.author << ", time: " << change.changetime
			<< ") in range (" << start << ", " << end << ")" << endl;
	}

	Range const newRange(start, end);

	ChangeTable::iterator it = table_.begin();

	for (; it != table_.end(); ) {
		// find super change, check for equal types, and do nothing
		if (it->range.contains(newRange) && it->change.type == change.type) {
			return;	
		}

		// current change starts like or follows new change
		if (it->range.start >= start) {
			break;
		}

		// new change intersects with existing change
		if (it->range.end > start) {
			pos_type oldEnd = it->range.end;
			it->range.end = start;
			if (lyxerr.debugging(Debug::CHANGES)) {
				lyxerr[Debug::CHANGES] << "  cutting tail of type " << it->change.type
					<< " resulting in range (" << it->range.start << ", "
					<< it->range.end << ")" << endl;
			}
			++it;
			if (oldEnd >= end) {
				if (lyxerr.debugging(Debug::CHANGES)) {
					lyxerr[Debug::CHANGES] << "  inserting tail in range ("
						<< end << ", " << oldEnd << ")" << endl;
				}
				it = table_.insert(it, ChangeRange((it-1)->change, Range(end, oldEnd)));
			}
			continue;
		}

		++it;
	}

	if (change.type != Change::UNCHANGED) {
		if (lyxerr.debugging(Debug::CHANGES)) {
			lyxerr[Debug::CHANGES] << "  inserting change" << endl;
		}
		it = table_.insert(it, ChangeRange(change, Range(start, end)));
		++it;
	}

	for (; it != table_.end(); ) {
		// new change 'contains' existing change
		if (newRange.contains(it->range)) {
			if (lyxerr.debugging(Debug::CHANGES)) {
				lyxerr[Debug::CHANGES] << "  removing subrange ("
					<< it->range.start << ", " << it->range.end << ")" << endl;
			}
			it = table_.erase(it);
			continue;
		}

		// new change precedes existing change
		if (it->range.start >= end) {
			break;
		}

		// new change intersects with existing change
		it->range.start = end;
		if (lyxerr.debugging(Debug::CHANGES)) {
			lyxerr[Debug::CHANGES] << "  cutting head of type "
				<< it->change.type << " resulting in range ("
				<< end << ", " << it->range.end << ")" << endl;
		}
		break; // no need for another iteration
	}

	merge();
}


void Changes::erase(pos_type const pos)
{
	if (lyxerr.debugging(Debug::CHANGES)) {
		lyxerr[Debug::CHANGES] << "Erasing change at position " << pos << endl;
	}

	ChangeTable::iterator it = table_.begin();
	ChangeTable::iterator end = table_.end();

	for (; it != end; ++it) {
		// range (pos,pos+x) becomes (pos,pos+x-1)
		if (it->range.start > pos) {
			--(it->range.start);
		}
		// range (pos-x,pos) stays (pos-x,pos)
		if (it->range.end > pos) {
			--(it->range.end);
		}
	}

	merge();
}


void Changes::insert(Change const & change, lyx::pos_type pos)
{
	if (lyxerr.debugging(Debug::CHANGES)) {
		lyxerr[Debug::CHANGES] << "Inserting change of type " << change.type
			<< " at position " << pos << endl;
	}

	ChangeTable::iterator it = table_.begin();
	ChangeTable::iterator end = table_.end();

	for (; it != end; ++it) {
		// range (pos,pos+x) becomes (pos+1,pos+x+1)
		if (it->range.start >= pos) {
			++(it->range.start);
		}

		// range (pos-x,pos) stays as it is
		if (it->range.end > pos) {
			++(it->range.end);
		}
	}

	set(change, pos, pos + 1); // set will call merge
}


Change const Changes::lookup(pos_type const pos) const
{
	if (table_.empty()) {
		return Change(Change::UNCHANGED);
	}
	ChangeTable::const_iterator it = table_.begin();
	ChangeTable::const_iterator const end = table_.end();

	for (; it != end; ++it) {
		if (it->range.contains(pos))
			return it->change;
	}

	return Change(Change::UNCHANGED);
}


bool Changes::isChanged(pos_type const start, pos_type const end) const
{
	ChangeTable::const_iterator it = table_.begin();
	ChangeTable::const_iterator const itend = table_.end();

	for (; it != itend; ++it) {
		if (it->range.intersects(Range(start, end))) {
			if (lyxerr.debugging(Debug::CHANGES)) {
				lyxerr[Debug::CHANGES] << "found intersection of range ("
					<< start << ", " << end << ") with ("
					<< it->range.start << ", " << it->range.end
					<< ") of type " << it->change.type << endl;
			}
			return true;
		}
	}
	return false;
}


void Changes::merge()
{
	if (lyxerr.debugging(Debug::CHANGES)) {
		lyxerr[Debug::CHANGES] << "merging changes..." << endl;
	}

	ChangeTable::iterator it = table_.begin();

	while (it != table_.end()) {
		if (lyxerr.debugging(Debug::CHANGES)) {
			lyxerr[Debug::CHANGES] << "  found change of type " << it->change.type
				<< " and range (" << it->range.start << ", " << it->range.end
				<< ")" << endl;
		}

		if (it->range.start == it->range.end) {
			if (lyxerr.debugging(Debug::CHANGES)) {
				lyxerr[Debug::CHANGES] << "  removing empty range for pos "
					<< it->range.start << endl;
			}

			table_.erase(it);
			// start again
			it = table_.begin();
			continue;
		}

		if (it + 1 == table_.end())
			break;

		if (it->change == (it + 1)->change && it->range.end == (it + 1)->range.start) {
			if (lyxerr.debugging(Debug::CHANGES)) {
				lyxerr[Debug::CHANGES] << "  merging ranges (" << it->range.start << ", "
					<< it->range.end << ") and (" << (it + 1)->range.start << ", "
					<< (it + 1)->range.end << ")" << endl;
			}
			(it + 1)->range.start = it->range.start;
			table_.erase(it);
			// start again
			it = table_.begin();
			continue;
		}

		++it;
	}
}


int Changes::latexMarkChange(odocstream & os,
			     Change::Type const old, Change::Type const change,
			     bool const & output)
{
	// FIXME: change tracking (MG)
	if (!output || old == change)
		return 0;

	static docstring const start(from_ascii("\\changestart{}"));
	static docstring const end(from_ascii("\\changeend{}"));
	static docstring const son(from_ascii("\\overstrikeon{}"));
	static docstring const soff(from_ascii("\\overstrikeoff{}"));

	int column = 0;

	if (old == Change::DELETED) {
		os << soff;
		column += soff.length();
	}

	switch (change) {
		case Change::UNCHANGED:
			os << end;
			column += end.length();
			break;

		case Change::DELETED:
			if (old == Change::UNCHANGED) {
				os << start;
				column += start.length();
			}
			os << son;
			column += son.length();
			break;

		case Change::INSERTED:
			if (old == Change::UNCHANGED) {
				os << start;
				column += start.length();
			}
			break;
	}

	return column;
}


void Changes::lyxMarkChange(std::ostream & os, int & column,
			    time_type const curtime,
			    Change const & old, Change const & change)
{
	// FIXME: change tracking (MG)
	if (old == change)
		return;

	column = 0;

	switch (change.type) {
		case Change::UNCHANGED:
			os << "\n\\change_unchanged\n";
			break;

		case Change::DELETED: {
			time_type t = change.changetime;
			if (!t)
				t = curtime;
			os << "\n\\change_deleted " << change.author
				<< " " << t << "\n";

			break;
		}

	case Change::INSERTED: {
			time_type t = change.changetime;
			if (!t)
				t = curtime;
			os << "\n\\change_inserted " << change.author
				<< " " << t << "\n";
			break;
	}
	}
}


} // namespace lyx
