/* \file iterators.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */


#include <config.h>

#include "iterators.h"
#include "paragraph.h"
#include "PosIterator.h"
#include "cursor.h"
#include "buffer.h"
#include "BufferView.h"
#include "dispatchresult.h"

#include "insets/inset.h"
#include "insets/updatableinset.h"
#include "insets/insettext.h"

#include <boost/next_prior.hpp>

using boost::next;

///
/// ParPosition
///


ParPosition::ParPosition(ParagraphList::iterator p, ParagraphList const & pl)
	: pit(p), plist(&pl)
{
	if (p != const_cast<ParagraphList&>(pl).end()) {
		it.reset(p->insetlist.begin());
	}
}


bool operator==(ParPosition const & pos1, ParPosition const & pos2)
{
	return pos1.pit == pos2.pit;
}


bool operator!=(ParPosition const & pos1, ParPosition const & pos2)
{
	return !(pos1 == pos2);
}


///
/// ParIterator
///

ParIterator::ParIterator(ParagraphList::iterator pit, ParagraphList const & pl)
{
	positions_.push_back(ParPosition(pit, pl));
}


ParIterator::~ParIterator()
{}


ParIterator::ParIterator(ParIterator const & pi)
	: positions_(pi.positions_)
{}


void ParIterator::operator=(ParIterator const & pi)
{
	ParIterator tmp(pi);
	swap(positions_ , tmp.positions_);
}


ParIterator & ParIterator::operator++()
{
	while (!positions_.empty()) {
		ParPosition & p = positions_.back();

		// Does the current inset contain more "cells" ?
		if (p.index) {
			++(*p.index);
			if (LyXText * text = (*p.it)->inset->getText(*p.index)) {
				ParagraphList & plist = text->paragraphs();
				if (!plist.empty()) {
					positions_.push_back(ParPosition(plist.begin(), plist));
					return *this;
				}
			}
			++(*p.it);
		} else {
			// The following line is needed because the value of
			// p.it may be invalid if inset was added/removed to
			// the paragraph pointed by the iterator
			p.it.reset(p.pit->insetlist.begin());
		}

		// Try to find the next inset that contains paragraphs
		InsetList::iterator end = p.pit->insetlist.end();
		for (; *p.it != end; ++(*p.it)) {
			if (LyXText * text = (*p.it)->inset->getText(0)) {
				ParagraphList & plist = text->paragraphs();
				if (!plist.empty()) {
					p.index.reset(0);
					positions_.push_back(ParPosition(plist.begin(), plist));
					return *this;
				}
			}
		}

		// Try to go to the next paragarph
		if (next(p.pit) != const_cast<ParagraphList*>(p.plist)->end()
		    || positions_.size() == 1) {
			++p.pit;
			p.index.reset();
			p.it.reset();

			return *this;
		}

		// Drop end and move up in the stack.
		positions_.pop_back();
	}
	return *this;
}


LyXText * ParIterator::text(Buffer & buf) const
{
	//lyxerr << "positions.size: " << positions.size() << std::endl;
	if (positions_.size() <= 1)
		return &buf.text();

	ParPosition const & pos = positions_[positions_.size() - 2];
	return (*pos.it)->inset->getText(*pos.index);
}


InsetBase * ParIterator::inset() const
{
	//lyxerr << "positions.size: " << positions.size() << std::endl;
	if (positions_.size() <= 1)
		return 0;

	ParPosition const & pos = positions_[positions_.size() - 2];
	return (*pos.it)->inset;
}


int ParIterator::index() const
{
	if (positions_.size() <= 1)
		return 0;

	return *(positions_[positions_.size() - 2].index);
}


Paragraph & ParIterator::operator*() const
{
	return *positions_.back().pit;
}


ParagraphList::iterator ParIterator::pit() const
{
	return positions_.back().pit;
}


ParagraphList::iterator ParIterator::operator->() const
{
	return positions_.back().pit;
}


ParagraphList::iterator ParIterator::outerPar() const
{
	return positions_[0].pit;
}


size_t ParIterator::size() const
{
	return positions_.size();
}


ParagraphList & ParIterator::plist() const
{
	return *const_cast<ParagraphList*>(positions_.back().plist);
}


ParIterator::ParIterator(PosIterator const & pos)
{
	int const size = pos.stack_.size();

	for (int i = 0; i < size; ++i) {
		PosIteratorItem const & it = pos.stack_[i];
		ParPosition pp(it.pit, *it.pl);
		if (i < size - 1) {
			InsetBase * inset = it.pit->getInset(it.pos);
			BOOST_ASSERT(inset);
			InsetList::iterator beg = it.pit->insetlist.begin();
			InsetList::iterator end = it.pit->insetlist.end();
			for ( ; beg != end && beg->inset != inset; ++beg)
				;
			pp.it.reset(beg);
			pp.index.reset(it.index - 1);
		}
		positions_.push_back(pp);
	}
}


void ParIterator::lockPath(BufferView * bv) const
{
	LCursor & cur = bv->cursor();
	cur.reset();
	int const last = size() - 1;
	for (int i = 0; i < last; ++i)
		(*positions_[i].it)->inset->edit(cur, true);
	cur.resetAnchor();
}


bool operator==(ParIterator const & iter1, ParIterator const & iter2)
{
	return iter1.positions() == iter2.positions();
}


bool operator!=(ParIterator const & iter1, ParIterator const & iter2)
{
	return !(iter1 == iter2);
}


///
/// ParConstIterator
///


ParConstIterator::ParConstIterator(ParagraphList::iterator pit,
				   ParagraphList const & pl)
{
	positions_.push_back(ParPosition(pit, pl));
}


ParConstIterator::~ParConstIterator()
{}


ParConstIterator::ParConstIterator(ParConstIterator const & pi)
	: positions_(pi.positions_)
{}


ParConstIterator & ParConstIterator::operator++()
{
	while (!positions_.empty()) {
		ParPosition & p = positions_.back();

		// Does the current inset contain more "cells" ?
		if (p.index) {
			++(*p.index);
			if (LyXText * text = (*p.it)->inset->getText(*p.index)) {
				ParagraphList & plist = text->paragraphs();
				if (!plist.empty()) {
					positions_.push_back(ParPosition(plist.begin(), plist));
					return *this;
				}
			}
			++(*p.it);
		} else {
			// The following line is needed because the value of
			// p.it may be invalid if inset was added/removed to
			// the paragraph pointed by the iterator
			p.it.reset(p.pit->insetlist.begin());
		}

		// Try to find the next inset that contains paragraphs
		InsetList::iterator end = p.pit->insetlist.end();
		for (; *p.it != end; ++(*p.it)) {
			if (LyXText * text = (*p.it)->inset->getText(0)) {
				ParagraphList & plist = text->paragraphs();
				if (!plist.empty()) {
					p.index.reset(0);
					positions_.push_back(ParPosition(plist.begin(), plist));
					return *this;
				}
			}
		}

		// Try to go to the next paragarph
		if (next(p.pit) != const_cast<ParagraphList*>(p.plist)->end()
		    || positions_.size() == 1) {
			++p.pit;
			p.index.reset();
			p.it.reset();

			return *this;
		}

		// Drop end and move up in the stack.
		positions_.pop_back();
	}

	return *this;
}


Paragraph const & ParConstIterator::operator*() const
{
	return *positions_.back().pit;
}


ParagraphList::const_iterator ParConstIterator::pit() const
{
	return positions_.back().pit;
}


ParagraphList::const_iterator ParConstIterator::operator->() const
{
	return positions_.back().pit;
}


ParagraphList const & ParConstIterator::plist() const
{
	return *positions_.back().plist;
}


size_t ParConstIterator::size() const
{
	return positions_.size();
}


bool operator==(ParConstIterator const & iter1, ParConstIterator const & iter2)
{
	return iter1.positions() == iter2.positions();
}


bool operator!=(ParConstIterator const & iter1, ParConstIterator const & iter2)
{
	return !(iter1 == iter2);
}
