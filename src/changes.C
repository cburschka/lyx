/**
 * \file changes.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
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


void Changes::set(Change const & change,
		  pos_type const start, pos_type const end)
{
	ChangeTable::iterator it = table_.begin();

	if (lyxerr.debugging(Debug::CHANGES)) {
		lyxerr[Debug::CHANGES] << "changeset of " << change.type
			<< " author " << change.author << " time " << change.changetime
			<< " in range " << start << "," << end << endl;
	}

	Range const new_range(start, end);

	// remove all sub-ranges
	for (; it != table_.end();) {
		if (new_range != it->range /*&& it->range.contained(new_range)*/) { // FIXME: change tracking (MG)
			if (lyxerr.debugging(Debug::CHANGES)) {
				lyxerr[Debug::CHANGES] << "Removing subrange "
					<< it->range.start << "," << it->range.end << endl;
			}
			it = table_.erase(it);
		} else {
			++it;
		}
	}

	it = table_.begin();
	ChangeTable::iterator const itend = table_.end();

	// find a super-range
	for (; it != itend; ++it) {
		if (it->range.contains(new_range))
			break;
	}

	if (it == itend) {
		lyxerr[Debug::CHANGES] << "Inserting change at end" << endl;
		table_.push_back(ChangeRange(change, Range(start, end)));
		merge();
		return;
	}

	if (change.type == it->change.type) {
		lyxerr[Debug::CHANGES] << "Change set already." << endl;
		it->change = change;
		return;
	}

	ChangeRange c(*it);

	if (lyxerr.debugging(Debug::CHANGES)) {
		lyxerr[Debug::CHANGES] << "Using change of type " << c.change.type
			<< " over " << c.range.start << "," << c.range.end << endl;
	}

	// split head
	if (c.range.start < start) {
		it = table_.insert(it, ChangeRange(c.change, Range(c.range.start, start)));
		if (lyxerr.debugging(Debug::CHANGES)) {
			lyxerr[Debug::CHANGES] << "Splitting head of type " << c.change.type
				<< " over " << c.range.start << "," << start << endl;
		}
		++it;
	}

	// reset this as new type
	it->range.start = start;
	it->range.end = end;
	it->change = change;
	lyxerr[Debug::CHANGES] << "Resetting to new change" << endl;

	// split tail
	if (c.range.end > end) {
		++it;
		table_.insert(it, ChangeRange(c.change, Range(end, c.range.end)));
		if (lyxerr.debugging(Debug::CHANGES)) {
			lyxerr[Debug::CHANGES] << "Splitting tail of type " << c.change.type
				<< " over " << end << "," << c.range.end << endl;
		}
	}

	merge();
}


void Changes::erase(pos_type const pos)
{
	ChangeTable::iterator it = table_.begin();
	ChangeTable::iterator end = table_.end();

	bool found = false;

	for (; it != end; ++it) {
		Range & range(it->range);

		if (lyxerr.debugging(Debug::CHANGES)) {
			lyxerr[Debug::CHANGES] << "era:Range of type " << it->change.type << " is "
				<< it->range.start << "," << it->range.end << endl;
		}

		if (range.contains(pos)) {
			found = true;
			--range.end;
			continue;
		}

		if (found) {
			--range.start;
			--range.end;
		}
	}
	merge();
}


Change const Changes::lookup(pos_type const pos) const
{
	ChangeTable::const_iterator it = table_.begin();
	ChangeTable::const_iterator const end = table_.end();

	for (; it != end; ++it) {
		if (it->range.contains(pos))
			return it->change;
	}

	BOOST_ASSERT(false && "missing changes for pos");
	return Change(Change::UNCHANGED);
}


bool Changes::isChanged(pos_type const start, pos_type const end) const
{
	ChangeTable::const_iterator it = table_.begin();
	ChangeTable::const_iterator const itend = table_.end();

	for (; it != itend; ++it) {
		if (lyxerr.debugging(Debug::CHANGES)) {
			lyxerr[Debug::CHANGES] << "Looking for " << start << ","
				<< end << " in " << it->range.start << ","
				<< it->range.end << "of type " << it->change.type << endl;
		}

		if (it->range.intersects(Range(start, end))
			&& it->change.type != Change::UNCHANGED) {
			if (lyxerr.debugging(Debug::CHANGES)) {
				lyxerr[Debug::CHANGES] << "Found intersection of "
					<< start << "," << end << " with "
					<< it->range.start << "," << it->range.end
					<< " of type " << it->change.type << endl;
			}
			return true;
		}
	}

	return false;
}


void Changes::merge()
{
	if (lyxerr.debugging(Debug::CHANGES))
		lyxerr[Debug::CHANGES] << "Starting merge" << endl;

	ChangeTable::iterator it = table_.begin();

	while (it != table_.end()) {
		if (lyxerr.debugging(Debug::CHANGES)) {
			lyxerr[Debug::CHANGES] << "Range of type " << it->change.type << " is "
				<< it->range.start << "," << it->range.end << endl;
		}

		if (it->range.start == it->range.end) {
			if (lyxerr.debugging(Debug::CHANGES)) {
				lyxerr[Debug::CHANGES] << "Removing empty range for pos "
					<< it->range.start << endl;
			}

			table_.erase(it);
			// start again
			it = table_.begin();
			continue;
		}

		if (it + 1 == table_.end())
			break;

		if (it->change == (it + 1)->change) {
			if (lyxerr.debugging(Debug::CHANGES)) {
				lyxerr[Debug::CHANGES] << "Merging equal ranges "
					<< it->range.start << "," << it->range.end
					<< " and " << (it + 1)->range.start << ","
					<< (it + 1)->range.end << endl;
			}

			(it + 1)->range.start = it->range.start;
			table_.erase(it);
			// start again
			it = table_.begin();
			continue;
		}

		++it;
	}

	lyxerr[Debug::CHANGES] << "Merge ended" << endl;
}


int Changes::latexMarkChange(odocstream & os,
			     Change::Type const old, Change::Type const change,
			     bool const & output)
{
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
