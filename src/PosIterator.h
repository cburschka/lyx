// -*- C++ -*-
/* \file PosIterator.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef POSITERATOR_H
#define POSITERATOR_H

#include "ParagraphList_fwd.h"

#include "iterators.h"

#include "support/types.h"

#include <vector>


class BufferView;

struct PosIteratorItem {
	PosIteratorItem(ParagraphList * pl,
			ParagraphList::iterator pit,
			lyx::pos_type pos,
			int index = 0)
		: pl(pl), pit(pit), pos(pos), index(index) {};
	ParagraphList * pl;
	ParagraphList::iterator pit;
	lyx::pos_type pos;
	int index;
};


class PosIterator : public std::iterator<
	std::bidirectional_iterator_tag,
        ParagraphList::value_type> {
public:
	// Creates a singular.
	PosIterator() {};

	PosIterator(BufferView & bv);
	PosIterator(ParagraphList * pl, ParagraphList::iterator pit,
		    lyx::pos_type pos);
	PosIterator(ParIterator const & par, lyx::pos_type pos);
	PosIterator & operator++();
	PosIterator & operator--();
	friend bool operator==(PosIterator const &, PosIterator const &);

	ParagraphList::iterator pit() const { return stack_.back().pit; }
	lyx::pos_type pos() const { return stack_.back().pos; }
	bool at_end() const;
	InsetBase * inset() const;
	friend ParIterator::ParIterator(PosIterator const &);
private:
	void setFrom(ParIterator const & par, lyx::pos_type pos);
	// This is conceptually a stack,
	// but we need random access sometimes.
	std::vector<PosIteratorItem> stack_;
};


bool operator==(PosIterator const &, PosIterator const &);


inline
bool operator!=(PosIterator const & lhs, PosIterator const & rhs)
{
	return !(lhs == rhs);
}

#endif
