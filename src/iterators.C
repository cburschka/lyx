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
#include <boost/optional.hpp>

using boost::next;
using boost::optional;
using std::vector;

///
/// ParPosition
///

class ParPosition {
public:
	///
	ParPosition(ParagraphList::iterator p, ParagraphList const & pl);
	///
	ParagraphList::iterator pit;
	///
	ParagraphList const * plist;
	///
	optional<InsetList::iterator> it;
	///
	optional<int> index;
};


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

struct ParIterator::Pimpl {
	typedef vector<ParPosition> PosHolder;
	PosHolder positions;
};

ParIterator::ParIterator(ParagraphList::iterator pit, ParagraphList const & pl)
	: pimpl_(new Pimpl)
{
	pimpl_->positions.push_back(ParPosition(pit, pl));
}


ParIterator::~ParIterator()
{}


ParIterator::ParIterator(ParIterator const & pi)
	: pimpl_(new Pimpl(*pi.pimpl_))
{}


void ParIterator::operator=(ParIterator const & pi)
{
	ParIterator tmp(pi);
	pimpl_.swap(tmp.pimpl_);
}


ParIterator & ParIterator::operator++()
{
	while (!pimpl_->positions.empty()) {
		ParPosition & p = pimpl_->positions.back();

		// Does the current inset contain more "cells" ?
		if (p.index) {
			++(*p.index);
			if (LyXText * text = (*p.it)->inset->getText(*p.index)) {
				ParagraphList & plist = text->paragraphs();
				if (!plist.empty()) {
					pimpl_->positions.push_back(ParPosition(plist.begin(), plist));
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
					pimpl_->positions.push_back(ParPosition(plist.begin(), plist));
					return *this;
				}
			}
		}

		// Try to go to the next paragarph
		if (next(p.pit) != const_cast<ParagraphList*>(p.plist)->end()
		    || pimpl_->positions.size() == 1) {
			++p.pit;
			p.index.reset();
			p.it.reset();

			return *this;
		}

		// Drop end and move up in the stack.
		pimpl_->positions.pop_back();
	}
	return *this;
}


LyXText * ParIterator::text(Buffer & buf) const
{
	//lyxerr << "positions.size: " << pimpl_->positions.size() << std::endl;
	if (pimpl_->positions.size() <= 1)
		return &buf.text();

	ParPosition const & pos = pimpl_->positions[pimpl_->positions.size() - 2];
	return (*pos.it)->inset->getText(*pos.index);
}


InsetOld * ParIterator::inset() const
{
	//lyxerr << "positions.size: " << pimpl_->positions.size() << std::endl;
	if (pimpl_->positions.size() <= 1)
		return 0;

	ParPosition const & pos = pimpl_->positions[pimpl_->positions.size() - 2];
	return (*pos.it)->inset;
}


int ParIterator::index() const
{
	if (pimpl_->positions.size() <= 1)
		return 0;

	return *(pimpl_->positions[pimpl_->positions.size() - 2].index);
}


Paragraph & ParIterator::operator*() const
{
	return *pimpl_->positions.back().pit;
}


ParagraphList::iterator ParIterator::pit() const
{
	return pimpl_->positions.back().pit;
}


ParagraphList::iterator ParIterator::operator->() const
{
	return pimpl_->positions.back().pit;
}


ParagraphList::iterator ParIterator::outerPar() const
{
	return pimpl_->positions[0].pit;
}


size_t ParIterator::size() const
{
	return pimpl_->positions.size();
}


ParagraphList & ParIterator::plist() const
{
	return *const_cast<ParagraphList*>(pimpl_->positions.back().plist);
}


bool operator==(ParIterator const & iter1, ParIterator const & iter2)
{
	return iter1.pimpl_->positions == iter2.pimpl_->positions;
}


bool operator!=(ParIterator const & iter1, ParIterator const & iter2)
{
	return !(iter1 == iter2);
}


///
/// ParConstIterator
///


struct ParConstIterator::Pimpl {
	typedef vector<ParPosition> PosHolder;
	PosHolder positions;
};


ParConstIterator::ParConstIterator(ParagraphList::iterator pit,
				   ParagraphList const & pl)
	: pimpl_(new Pimpl)
{
	pimpl_->positions.push_back(ParPosition(pit, pl));
}


ParConstIterator::~ParConstIterator()
{}


ParConstIterator::ParConstIterator(ParConstIterator const & pi)
	: pimpl_(new Pimpl(*pi.pimpl_))
{}


ParConstIterator & ParConstIterator::operator++()
{
	while (!pimpl_->positions.empty()) {
		ParPosition & p = pimpl_->positions.back();

		// Does the current inset contain more "cells" ?
		if (p.index) {
			++(*p.index);
			if (LyXText * text = (*p.it)->inset->getText(*p.index)) {
				ParagraphList & plist = text->paragraphs();
				if (!plist.empty()) {
					pimpl_->positions.push_back(ParPosition(plist.begin(), plist));
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
					pimpl_->positions.push_back(ParPosition(plist.begin(), plist));
					return *this;
				}
			}
		}

		// Try to go to the next paragarph
		if (next(p.pit) != const_cast<ParagraphList*>(p.plist)->end()
		    || pimpl_->positions.size() == 1) {
			++p.pit;
			p.index.reset();
			p.it.reset();

			return *this;
		}

		// Drop end and move up in the stack.
		pimpl_->positions.pop_back();
	}

	return *this;
}


Paragraph const & ParConstIterator::operator*() const
{
	return *pimpl_->positions.back().pit;
}


ParagraphList::const_iterator ParConstIterator::pit() const
{
	return pimpl_->positions.back().pit;
}


ParagraphList::const_iterator ParConstIterator::operator->() const
{
	return pimpl_->positions.back().pit;
}


ParagraphList const & ParConstIterator::plist() const
{
	return *pimpl_->positions.back().plist;
}


size_t ParConstIterator::size() const
{
	return pimpl_->positions.size();
}


bool operator==(ParConstIterator const & iter1, ParConstIterator const & iter2)
{
	return iter1.pimpl_->positions == iter2.pimpl_->positions;
}


bool operator!=(ParConstIterator const & iter1, ParConstIterator const & iter2)
{
	return !(iter1 == iter2);
}


PosIterator ParIterator::asPosIterator(lyx::pos_type pos) const
{
	PosIterator p;

	int const last = size() - 1;
	for (int i = 0; i < last; ++i) {
		ParPosition & pp = pimpl_->positions[i];
		p.stack_.push_back(
			PosIteratorItem(const_cast<ParagraphList *>(pp.plist),
				pp.pit, (*pp.it)->pos, *pp.index + 1));
	}
	ParPosition const & pp = pimpl_->positions[last];
	p.stack_.push_back(
		PosIteratorItem(const_cast<ParagraphList *>(pp.plist), pp.pit, pos, 0));
	return p;
}


ParIterator::ParIterator(PosIterator const & pos)
	: pimpl_(new Pimpl)
{
	int const size = pos.stack_.size();

	for (int i = 0; i < size; ++i) {
		PosIteratorItem const & it = pos.stack_[i];
		ParPosition pp(it.pit, *it.pl);
		if (i < size - 1) {
			InsetOld * inset = it.pit->getInset(it.pos);
			BOOST_ASSERT(inset);
			InsetList::iterator beg = it.pit->insetlist.begin();
			InsetList::iterator end = it.pit->insetlist.end();
			for ( ; beg != end && beg->inset != inset; ++beg)
				;
			pp.it.reset(beg);
			pp.index.reset(it.index - 1);
		}
		pimpl_->positions.push_back(pp);
	}
}


void ParIterator::lockPath(BufferView * bv) const
{
	bv->fullCursor() = LCursor(bv);
	int last = size() - 1;
#warning this seems to create just one entry for InsetTabulars
	for (int i = 0; i < last; ++i)
		(*pimpl_->positions[i].it)->inset->edit(bv, true);
}
