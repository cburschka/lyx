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


struct Change {
	/// the type of change
	enum Type {
		UNCHANGED, // no change
		INSERTED, // new text
		DELETED // deleted text
	};

	Change(Type t = UNCHANGED, int a = 0, lyx::time_type ct = 0)
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
	void set(Change change, lyx::pos_type pos);

	/// set the position to the given change
	void set(Change::Type, lyx::pos_type pos);

	/// set the range to the given change
	void set(Change::Type, lyx::pos_type start, lyx::pos_type end);

	/// set the range to the given change
	void set(Change, lyx::pos_type start, lyx::pos_type end);

	/// mark the given change and adjust
	void record(Change, lyx::pos_type pos);

	/// return the change type at the given position
	Change::Type lookup(lyx::pos_type pos) const;

	/// return the change at the given position
	Change const lookupFull(lyx::pos_type pos) const;

	/// return true if there is a change in the given range
	bool isChange(lyx::pos_type start, lyx::pos_type end) const;

	/// return true if there is a deleted or unchanged range contained
	bool isChangeEdited(lyx::pos_type start, lyx::pos_type end) const;

	/// remove the given entry
	void erase(lyx::pos_type pos);

	/// output latex to mark a transition between two changetypes
	/// returns length of text outputted
	static int latexMarkChange(std::ostream & os, Change::Type old, Change::Type change);

	/// output .lyx file format for transitions between changes
	static void lyxMarkChange(std::ostream & os, int & column,
		lyx::time_type curtime, Change const & old, Change const & change);

private:
	struct Range {
		Range(lyx::pos_type s, lyx::pos_type e)
			: start(s), end(e) {}

		// does this range contain r ?
		bool contains(Range const & r) const;

		// does this range contain pos ?
		bool contains(lyx::pos_type pos) const;

		// does this range contain pos, or can it be appended ?
		bool loose_contains(lyx::pos_type pos) const;

		// is this range contained within r ?
		bool contained(Range const & r) const;

		// do the ranges intersect ?
		bool intersects(Range const & r) const;

		lyx::pos_type start;
		lyx::pos_type end;
	};

	friend bool operator==(Range const & r1, Range const & r2);
	friend bool operator!=(Range const & r1, Range const & r2);

	struct ChangeRange {
		ChangeRange(lyx::pos_type s, lyx::pos_type e, Change c)
			: range(Range(s, e)), change(c) {}
		Range range;
		Change change;
	};

	typedef std::vector<ChangeRange> ChangeTable;

	/// our table of changes
	ChangeTable table_;

	/// change type for an empty paragraph
	Change::Type empty_type_;

	/// handle a delete
	void del(Change change, ChangeTable::size_type pos);

	/// handle an add
	void add(Change change, ChangeTable::size_type pos);

	/// merge neighbouring ranges
	void merge();

	/// consistency check
	void check() const;

};

#endif // CHANGES_H
