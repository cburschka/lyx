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

#include <stack>


class BufferView;

struct PosIteratorItem 
{
	PosIteratorItem(ParagraphList * pl): pl(pl), pit(pl->begin()),
					     pos(0), index(0) {};
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


class PosIterator
{
public:
	PosIterator(BufferView & bv);
	PosIterator(ParIterator & par, lyx::pos_type pos);
	PosIterator(ParagraphList * pl);
	PosIterator(ParagraphList * pl, ParagraphList::iterator pit,
		    lyx::pos_type pos);
	PosIterator(ParIterator const & parit, lyx::pos_type p);
	PosIterator & operator++();
	PosIterator & operator--();
	friend bool operator==(PosIterator const &, PosIterator const &);

	ParagraphList::iterator pit() const { return stack_.top().pit; }
	lyx::pos_type pos() const { return stack_.top().pos; }
	bool at_end() const;
	friend PosIterator ParIterator::asPosIterator(lyx::pos_type) const;
	
private:
	PosIterator() {};
	std::stack<PosIteratorItem> stack_;
};

bool operator!=(PosIterator const &, PosIterator const &);
bool operator==(PosIterator const &, PosIterator const &);

int distance(PosIterator const &, PosIterator const &);
void advance(PosIterator &, int);

#endif

