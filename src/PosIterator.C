/* \file PosIterator.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */


#include <config.h>

#include "PosIterator.h"

#include "buffer.h"
#include "BufferView.h"
#include "cursor.h"
#include "iterators.h"
#include "lyxtext.h"
#include "paragraph.h"

#include "insets/insettext.h"
#include "insets/updatableinset.h"
#include "insets/inset.h"

#include <boost/next_prior.hpp>

using boost::prior;


PosIterator & PosIterator::operator++()
{
	BOOST_ASSERT(!stack_.empty());
	while (true) {
		PosIteratorItem & p = stack_.back();

		if (p.pos < p.pit->size()) {
			if (InsetBase * inset = p.pit->getInset(p.pos)) {
				if (LyXText * text = inset->getText(p.index)) {
					ParagraphList & pl = text->paragraphs();
					p.index++;
					stack_.push_back(PosIteratorItem(&pl, pl.begin(), 0));
					return *this;
				}
			}
			p.index = 0;
			++p.pos;
		} else {
			++p.pit;
			p.pos = 0;
		}

		if (p.pit != p.pl->end() || stack_.size() == 1)
			return *this;

		stack_.pop_back();
	}
	return *this;
}


PosIterator & PosIterator::operator--()
{
	BOOST_ASSERT(!stack_.empty());

	// try to go one position backwards: if on the start of the
	// ParagraphList, pops an item
	PosIteratorItem & p = stack_.back();
	if (p.pos > 0) {
		--p.pos;
		InsetBase * inset = p.pit->getInset(p.pos);
		if (inset)
			p.index = inset->numParagraphs();
	} else {
		if (p.pit == p.pl->begin()) {
			if (stack_.size() == 1)
				return *this;
			stack_.pop_back();
			--stack_.back().index;
		} else {
			--p.pit;
			p.pos = p.pit->size();
		}
	}
	// try to push an item if there is some left unexplored
	PosIteratorItem & q = stack_.back();
	if (q.pos < q.pit->size()) {
		InsetBase * inset = q.pit->getInset(q.pos);
		if (inset && q.index > 0) {
			LyXText * text = inset->getText(q.index - 1);
			BOOST_ASSERT(text);
			ParagraphList & pl = text->paragraphs();
			stack_.push_back(PosIteratorItem(&pl, prior(pl.end()), pl.back().size()));
		}
	}
	return *this;
}


bool operator!=(PosIterator const & lhs, PosIterator const & rhs)
{
	return !(lhs == rhs);
}


bool operator==(PosIterator const & lhs, PosIterator const & rhs)
{

	PosIteratorItem const & li = lhs.stack_.back();
	PosIteratorItem const & ri = rhs.stack_.back();

	return (li.pl == ri.pl && li.pit == ri.pit &&
		(li.pit == li.pl->end() || li.pos == ri.pos));
}


bool PosIterator::at_end() const
{
	return pos() == pit()->size();
}


PosIterator::PosIterator(ParagraphList * pl, ParagraphList::iterator pit,
			 lyx::pos_type pos)
{
	stack_.push_back(PosIteratorItem(pl, pit, pos));
}


PosIterator::PosIterator(BufferView & bv)
{
	LyXText * text = bv.getLyXText();
	lyx::pos_type pos = bv.cursor().pos();
	ParagraphList::iterator pit = text->cursorPar();

	ParIterator par = bv.buffer()->par_iterator_begin();
	ParIterator end = bv.buffer()->par_iterator_end();
	for ( ; par != end; ++par) {
		if (par.pit() == pit)
			break;
	}

	operator=(par.asPosIterator(pos));
}


InsetBase * PosIterator::inset() const
{
	if (stack_.size() == 1)
		return 0;
	PosIteratorItem const & pi = stack_[stack_.size() - 2];
	return pi.pit->getInset(pi.pos);
}
