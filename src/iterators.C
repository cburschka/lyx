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

#include "insets/inset.h"

#include <boost/next_prior.hpp>
#include <boost/optional.hpp>

// it's conceptionally a stack, but undo needs random access...
//#include <stack>

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
			ParagraphList * plist = (*p.it)->inset->getParagraphs(*p.index);
			if (plist && !plist->empty()) {
				pimpl_->positions.push_back(ParPosition(plist->begin(), *plist));
				return *this;
			}
			++(*p.it);
		} else
			// The following line is needed because the value of
			// p.it may be invalid if inset was added/removed to
			// the paragraph pointed by the iterator
			p.it.reset(p.pit->insetlist.begin());

		// Try to find the next inset that contains paragraphs
		InsetList::iterator end = p.pit->insetlist.end();
		for (; *p.it != end; ++(*p.it)) {
			ParagraphList * plist = (*p.it)->inset->getParagraphs(0);
			if (plist && !plist->empty()) {
				p.index.reset(0);
				pimpl_->positions.push_back(ParPosition(plist->begin(), *plist));
				return *this;
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
			ParagraphList * plist = (*p.it)->inset->getParagraphs(*p.index);
			if (plist && !plist->empty()) {
				pimpl_->positions.push_back(ParPosition(plist->begin(), *plist));
				return *this;
			}
			++(*p.it);
		} else
			// The following line is needed because the value of
			// p.it may be invalid if inset was added/removed to
			// the paragraph pointed by the iterator
			p.it.reset(p.pit->insetlist.begin());

		// Try to find the next inset that contains paragraphs
		InsetList::iterator end = p.pit->insetlist.end();
		for (; *p.it != end; ++(*p.it)) {
			ParagraphList * plist = (*p.it)->inset->getParagraphs(0);
			if (plist && !plist->empty()) {
				p.index.reset(0);
				pimpl_->positions.push_back(ParPosition(plist->begin(), *plist));
				return *this;
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
