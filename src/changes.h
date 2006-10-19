// -*- C++ -*-
/**
 * \file changes.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 *
 * Record changes in a paragraph.
 */

#ifndef CHANGES_H
#define CHANGES_H

#include "support/types.h"
#include "support/lyxtime.h"

#include <vector>
#include <iosfwd>


class Change {
public:
	/// the type of change
	enum Type {
		UNCHANGED, // no change
		INSERTED, // new text
		DELETED // deleted text
	};

	explicit Change(Type t, int a = 0, lyx::time_type ct = 0)
		: type(t), author(a), changetime(ct) {}

	Type type;

	int author;

	lyx::time_type changetime;
};

bool operator==(Change const & l, Change const & r);
bool operator!=(Change const & l, Change const & r);

class Changes {
public:

	Changes(Change::Type type);

	~Changes();

	Changes(Changes const &);

	/// reset "default" change type (for empty pars)
	void reset(Change::Type type) {
		empty_type_ = type;
	}

	/// set the position to the given change
	void set(Change const & change, lyx::pos_type pos);

	/// set the position to the given change
	void set(Change::Type, lyx::pos_type pos);

	/// set the range to the given change
	void set(Change::Type, lyx::pos_type start, lyx::pos_type end);

	/// set the range to the given change
	void set(Change const & change, lyx::pos_type start, lyx::pos_type end);

	/// mark the given change and adjust
	void record(Change const & change, lyx::pos_type pos);

	/// return the change at the given position
	Change const lookup(lyx::pos_type pos) const;

	/// return true if there is a change in the given range
	bool isChange(lyx::pos_type start, lyx::pos_type end) const;

	/// return true if there is a deleted or unchanged range contained
	bool isChangeEdited(lyx::pos_type start, lyx::pos_type end) const;

	/// remove the given entry. This implies that a character was
	/// deleted at pos, and will adjust all range bounds past it
	void erase(lyx::pos_type pos);

	/// output latex to mark a transition between two changetypes
	/// returns length of text outputted
	static int latexMarkChange(std::ostream & os, Change::Type old,
		Change::Type change, bool const & output);

	/// output .lyx file format for transitions between changes
	static void lyxMarkChange(std::ostream & os, int & column,
		lyx::time_type curtime, Change const & old, Change const & change);

private:
	class Range {
	public:
		Range(lyx::pos_type s, lyx::pos_type e)
			: start(s), end(e) {}

		// does this range contain r ?
		bool contains(Range const & r) const;

		// does this range contain pos ?
		bool contains(lyx::pos_type pos) const;

		// does this range contain pos, or can it be appended ?
		bool containsOrPrecedes(lyx::pos_type pos) const;

		// is this range contained within r ?
		bool contained(Range const & r) const;

		// do the ranges intersect ?
		bool intersects(Range const & r) const;

		lyx::pos_type start;
		lyx::pos_type end;
	};

	friend bool operator==(Range const & r1, Range const & r2);
	friend bool operator!=(Range const & r1, Range const & r2);

	class ChangeRange {
	public:
		ChangeRange(lyx::pos_type s, lyx::pos_type e, Change const & c)
			: range(Range(s, e)), change(c) {}
		Range range;
		Change change;
	};

	typedef std::vector<ChangeRange> ChangeTable;

	/// our table of changes, every row a range and change descriptor
	ChangeTable table_;

	/// change type for an empty paragraph
	Change::Type empty_type_;

	/// handle a delete, either logical or physical (see erase)
	void del(Change const & change, ChangeTable::size_type pos);

	/// handle an add, adjusting range bounds past it
	void add(Change const & change, ChangeTable::size_type pos);

	/// merge neighbouring ranges, assuming that they are abutting
	/// (as done by set())
	void merge();

	/// consistency check, needed before merge()
	void check() const;

};

#endif // CHANGES_H
