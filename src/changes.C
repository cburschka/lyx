/**
 * \file changes.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * Record changes in a paragraph.
 *
 * \author John Levon <levon@movementarian.org>
 */

#include <config.h>

#include "changes.h"
#include "debug.h"
#include "author.h"

#include "support/LAssert.h"
#include "support/LOstream.h"

using std::vector;
using std::endl;
using lyx::pos_type;

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


bool Changes::Range::contained(Range const & r) const
{
	return r.contains(*this);
}


bool Changes::Range::contains(pos_type pos) const
{
	return pos >= start && pos < end;
}


bool Changes::Range::loose_contains(pos_type pos) const
{
	return pos >= start && pos <= end;
}


bool Changes::Range::intersects(Range const & r) const
{
	return contained(r) || contains(r)
		|| contains(r.start) || contains(r.end);
}


Changes::Changes(Change::Type type)
	: empty_type_(type)
{
}


Changes::~Changes()
{
}


Changes::Changes(Changes const & c)
{
	table_ = c.table_;
}


void Changes::record(Change change, pos_type pos)
{
	if (lyxerr.debugging(Debug::CHANGES)) {
		lyxerr[Debug::CHANGES] << "record " << change.type
			<< " at pos " << pos << " with total "
			<< table_.size() << " changes." << endl;
	}

	switch (change.type) {
		case Change::INSERTED:
			add(change, pos);
			break;
		case Change::DELETED:
			del(change, pos);
			break;
		case Change::UNCHANGED:
			set(Change::UNCHANGED, pos);
			break;
	}
}


void Changes::set(Change change, pos_type pos)
{
	set(change, pos, pos + 1);
}


void Changes::set(Change::Type type, pos_type pos)
{
	set(type, pos, pos + 1);
}


void Changes::set(Change::Type type, pos_type start, pos_type end)
{
	set(Change(type), start, end);
}


void Changes::set(Change change, pos_type start, pos_type end)
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
		if (new_range != it->range && it->range.contained(new_range)) {
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
	ChangeTable::iterator itend = table_.end();

	// find a super-range
	for (; it != itend; ++it) {
		if (it->range.contains(new_range))
			break;
	}

	if (it == itend) {
		lyxerr[Debug::CHANGES] << "Inserting change at end" << endl;
		table_.push_back(ChangeRange(start, end, change));
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
		it = table_.insert(it, ChangeRange(c.range.start, start, c.change));
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
		table_.insert(it, ChangeRange(end, c.range.end, c.change));
		if (lyxerr.debugging(Debug::CHANGES)) {
			lyxerr[Debug::CHANGES] << "Splitting tail of type " << c.change.type
				<< " over " << end << "," << c.range.end << endl;
		}
	}

	check();
	merge();
}


void Changes::erase(pos_type pos)
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
	check();
	merge();
}


void Changes::del(Change change, ChangeTable::size_type pos)
{
	// this case happens when building from .lyx
	if (table_.empty()) {
		set(change, pos);
		return;
	}

	ChangeTable::iterator it = table_.begin();

	for (; it != table_.end(); ++it) {
		Range & range(it->range);

		if (range.contains(pos)) {
			if (it->change.type != Change::INSERTED) {
				set(change, pos);
			} else {
				erase(pos);
			}
			break;
		} else if (range.loose_contains(pos) && it + 1 == table_.end()) {
			// this case happens when building from .lyx
			set(change, pos);
			break;
		}
	}
}


void Changes::add(Change change, ChangeTable::size_type pos)
{
	ChangeTable::iterator it = table_.begin();
	ChangeTable::iterator end = table_.end();

	bool found = false;

	for (; it != end; ++it) {
		Range & range(it->range);

		if (!found && range.loose_contains(pos)) {
			found = true;
			if (lyxerr.debugging(Debug::CHANGES)) {
				lyxerr[Debug::CHANGES] << "Found range of "
					<< range.start << "," << range.end << endl;
			}
			++range.end;
			continue;
		}

		if (found) {
			++range.start;
			++range.end;
		}
	}
	set(change, pos);
}


Change const Changes::lookupFull(pos_type pos) const
{
	if (!table_.size()) {
		if (lyxerr.debugging(Debug::CHANGES))
			lyxerr[Debug::CHANGES] << "Empty, type is " << empty_type_ << endl;
		return Change(empty_type_);
	}

	ChangeTable::const_iterator it = table_.begin();
	ChangeTable::const_iterator end = table_.end();

	for (; it != end; ++it) {
		if (it->range.contains(pos))
			return it->change;
	}

	check();
	lyx::Assert(0);
	return Change(Change::UNCHANGED);
}


Change::Type Changes::lookup(pos_type pos) const
{
	if (!table_.size()) {
		if (lyxerr.debugging(Debug::CHANGES))
			lyxerr[Debug::CHANGES] << "Empty, type is " << empty_type_ << endl;
		return empty_type_;
	}

	ChangeTable::const_iterator it = table_.begin();
	ChangeTable::const_iterator end = table_.end();

	for (; it != end; ++it) {
		if (it->range.contains(pos))
			return it->change.type;
	}

	check();
	lyx::Assert(0);
	return Change::UNCHANGED;
}


bool Changes::isChange(pos_type start, pos_type end) const
{
	if (!table_.size()) {
		if (lyxerr.debugging(Debug::CHANGES))
			lyxerr[Debug::CHANGES] << "Empty, type is " << empty_type_ << endl;
		return empty_type_ != Change::UNCHANGED;
	}

	ChangeTable::const_iterator it = table_.begin();
	ChangeTable::const_iterator itend = table_.end();

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


bool Changes::isChangeEdited(lyx::pos_type start, lyx::pos_type end) const
{
	if (!table_.size()) {
		if (lyxerr.debugging(Debug::CHANGES))
			lyxerr[Debug::CHANGES] << "Empty, type is " << empty_type_ << endl;
		return empty_type_ != Change::INSERTED;
	}

	ChangeTable::const_iterator it = table_.begin();
	ChangeTable::const_iterator itend = table_.end();

	for (; it != itend; ++it) {
		if (it->range.intersects(Range(start, end ? end - 1 : 0))
			&& it->change.type != Change::INSERTED) {
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
	check();
}


void Changes::check() const
{
	ChangeTable::const_iterator it = table_.begin();
	ChangeTable::const_iterator end = table_.end();

	bool dont_assert(true);

	lyxerr[Debug::CHANGES] << "Changelist:" << endl;
	for (; it != end; ++it) {
		if (lyxerr.debugging(Debug::CHANGES)) {
			lyxerr[Debug::CHANGES] << "Range of type " << it->change.type << " is "
				<< it->range.start << "," << it->range.end << " author "
				<< it->change.author << " time " << it->change.changetime << endl;
		}

		if (it + 1 == end)
			break;

		Range const & range(it->range);
		Range const & next((it + 1)->range);
		if (range.end != next.start)
			dont_assert = false;
	}

	if (lyxerr.debugging(Debug::CHANGES))
		lyxerr[Debug::CHANGES] << "End" << endl;

	lyx::Assert(dont_assert);
}


int Changes::latexMarkChange(std::ostream & os, Change::Type old, Change::Type change)
{
	if (old == change)
		return 0;

	string const start("\\changestart{}");
	string const end("\\changeend{}");
	string const son("\\overstrikeon{}");
	string const soff("\\overstrikeoff{}");

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


void Changes::lyxMarkChange(std::ostream & os, int & column, lyx::time_type curtime,
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
			lyx::time_type t(change.changetime);
			if (!t)
				t = curtime;
			os << "\n\\change_deleted " << change.author
				<< " " << t << "\n";

			break;
		}

		case Change::INSERTED:
			lyx::time_type t(change.changetime);
			if (!t)
				t = curtime;
			os << "\n\\change_inserted " << change.author
				<< " " << t << "\n";
			break;
	}
}
