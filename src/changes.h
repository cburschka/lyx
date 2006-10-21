// -*- C++ -*-
/**
 * \file changes.h
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

#ifndef CHANGES_H
#define CHANGES_H

#include "support/docstream.h"
#include "support/lyxtime.h"

#include <vector>


namespace lyx {


class Change {
public:
	/// the type of change
	enum Type {
		UNCHANGED, // no change
		INSERTED, // new text
		DELETED // deleted text
	};

	explicit Change(Type t, int a = 0, time_type ct = 0)
		: type(t), author(a), changetime(ct) {}

	Type type;

	int author;

	time_type changetime;
};

bool operator==(Change const & l, Change const & r);
bool operator!=(Change const & l, Change const & r);

class Changes {
public:
	/// set the pos to the given change
	void set(Change const & change, pos_type pos);
	/// set the range to the given change
	void set(Change const & change, pos_type start, pos_type end);

	/// return the change at the given position
	Change const lookup(pos_type pos) const;

	/// return true if there is a change in the given range
	bool isChanged(pos_type start, pos_type end) const;

	/// remove the given entry. This implies that a character was
	/// deleted at pos, and will adjust all range bounds past it
	void erase(pos_type pos);

	/// output latex to mark a transition between two changetypes
	/// returns length of text outputted
	static int latexMarkChange(odocstream & os, Change::Type old,
		Change::Type change, bool const & output);

	/// output .lyx file format for transitions between changes
	static void lyxMarkChange(std::ostream & os, int & column,
		time_type curtime, Change const & old, Change const & change);

private:
	class Range {
	public:
		Range(pos_type s, pos_type e)
			: start(s), end(e) {}

		// does this range contain r ?
		bool contains(Range const & r) const;

		// does this range contain pos ?
		bool contains(pos_type pos) const;

		// do the ranges intersect ?
		bool intersects(Range const & r) const;

		pos_type start;
		pos_type end; // Caution: end is not in the range!
	};

	friend bool operator==(Range const & r1, Range const & r2);
	friend bool operator!=(Range const & r1, Range const & r2);

	class ChangeRange {
	public:
		ChangeRange(pos_type s, pos_type e, Change const & c)
			: range(Range(s, e)), change(c) {}
		Range range;
		Change change;
	};

	/// merge neighbouring ranges, assuming that they are abutting
	void merge();

	typedef std::vector<ChangeRange> ChangeTable;

	/// our table of changes, every row a range and change descriptor
	ChangeTable table_;
};


} // namespace lyx

#endif // CHANGES_H
