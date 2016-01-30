// -*- C++ -*-
/**
 * \file Changes.h
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

#include "Color.h"

#include "support/strfwd.h"
#include "support/types.h"
#include "support/lyxtime.h"

#include "texstream.h"

#include <vector>


namespace lyx {

class AuthorList;
class Buffer;
class DocIterator;
class OutputParams;

class Change {
public:
	/// the type of change
	enum Type {
		UNCHANGED, // no change
		INSERTED, // new text
		DELETED // deleted text
	};

	explicit Change(Type t = UNCHANGED, int a = 0, time_t ct = support::current_time())
		: type(t), author(a), changetime(ct) {}

	/// is the change similar to the given change such that both can be merged?
	bool isSimilarTo(Change const & change) const;
	/// The color of this change on screen
	Color color() const;
	///
	bool changed() const { return type != UNCHANGED; }
	///
	void setUnchanged() { type = UNCHANGED; }
	///
	bool inserted() const { return type == INSERTED; }
	///
	void setInserted() { type = INSERTED; }
	///
	bool deleted() const { return type == DELETED; }
	///
	void setDeleted() { type = DELETED; }
	/// Is this change made by the current author ?
	bool currentAuthor() const { return author == 0; }

	Type type;

	int author;

	time_t changetime;
};

bool operator==(Change const & l, Change const & r);
bool operator!=(Change const & l, Change const & r);

class BufferParams;

class Changes {
public:
	Changes() : is_update_required_(false) {}

	/// set the pos to the given change
	void set(Change const & change, pos_type pos);
	/// set the range (excluding end) to the given change
	void set(Change const & change, pos_type start, pos_type end);

	/// erase the entry at pos and adjust all range bounds past it
	/// (assumes that a character was deleted at pos)
	void erase(lyx::pos_type pos);

	/// insert a new entry at pos and adjust all range bounds past it
	/// (assumes that a character was inserted at pos)
	void insert(Change const & change, lyx::pos_type pos);

	///

	/// return the change at the given pos
	Change const & lookup(pos_type pos) const;

	/// return true if there is a change in the given range (excluding end)
	bool isChanged(pos_type start, pos_type end) const;
	///
	bool isChanged() const;

	/// return true if the whole range is deleted
	bool isDeleted(pos_type start, pos_type end) const;

	/// output latex to mark a transition between two change types
	/// returns length of text outputted
	static int latexMarkChange(otexstream & os, BufferParams const & bparams,
				   Change const & oldChange, Change const & change,
				   OutputParams const & runparams);

	/// output .lyx file format for transitions between changes
	static void lyxMarkChange(std::ostream & os, BufferParams const & bparams,
		int & column, Change const & old, Change const & change);

	///
	void checkAuthors(AuthorList const & authorList);

	///
	void addToToc(DocIterator const & cdit, Buffer const & buffer,
	        bool output_active) const;

	///
	void updateBuffer(Buffer const & buf);
	///
	bool isUpdateRequired() const { return is_update_required_; }

private:
	class Range {
	public:
		Range(pos_type s, pos_type e)
			: start(s), end(e) {}

		// does this range contain r ? (inlined as the result of profiling)
		bool contains(Range const & r) const {
			return r.start >= start && r.end <= end;
		}

		// does this range contain pos ? (inlined as the result of profiling)
		bool contains(pos_type pos) const {
			return pos >= start && pos < end;
		}

		// do the ranges intersect ?
		bool intersects(Range const & r) const;

		pos_type start;
		pos_type end; // Caution: end is not in the range!
	};

	friend bool operator==(Range const & r1, Range const & r2);
	friend bool operator!=(Range const & r1, Range const & r2);

	class ChangeRange {
	public:
		ChangeRange(Change const & c, Range const & r)
			: change(c), range(r) {}

		Change change;
		Range range;
	};

	/// merge equal changes with adjoining ranges
	void merge();

	typedef std::vector<ChangeRange> ChangeTable;

	/// table of changes, every row a change and range descriptor
	ChangeTable table_;

	/// signals that the buffer's flag tracked_changes_present_ needs to be
	/// recalculated
	bool is_update_required_;
};


} // namespace lyx

#endif // CHANGES_H
