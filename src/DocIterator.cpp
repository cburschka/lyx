/**
 * \file DocIterator.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */


#include <config.h>

#include "DocIterator.h"

#include "debug.h"
#include "LyXText.h"
#include "Paragraph.h"

#include "mathed/MathData.h"
#include "mathed/InsetMath.h"

#include "insets/InsetTabular.h"

#include <boost/assert.hpp>
#include <boost/current_function.hpp>


namespace lyx {

using std::endl;


// We could be able to get rid of this if only every BufferView were
// associated to a buffer on construction.
DocIterator::DocIterator()
	: boundary_(false), inset_(0)
{}


DocIterator::DocIterator(InsetBase & inset)
	: boundary_(false), inset_(&inset)
{}


DocIterator doc_iterator_begin(InsetBase & inset)
{
	DocIterator dit(inset);
	dit.forwardPos();
	return dit;
}


DocIterator doc_iterator_end(InsetBase & inset)
{
	return DocIterator(inset);
}


InsetBase * DocIterator::nextInset()
{
	BOOST_ASSERT(!empty());
	if (pos() == lastpos())
		return 0;
	if (pos() > lastpos()) {
		lyxerr << "Should not happen, but it does. " << endl;
		return 0;
	}
	if (inMathed())
		return nextAtom().nucleus();
	return paragraph().isInset(pos()) ? paragraph().getInset(pos()) : 0;
}


InsetBase * DocIterator::prevInset()
{
	BOOST_ASSERT(!empty());
	if (pos() == 0)
		return 0;
	if (inMathed())
		if (cell().empty())
			// FIXME: this should not happen but it does.
			// See bug 3189
			// http://bugzilla.lyx.org/show_bug.cgi?id=3189
			return 0;
		else
			return prevAtom().nucleus();
	return paragraph().isInset(pos() - 1) ? paragraph().getInset(pos() - 1) : 0;
}


InsetBase const * DocIterator::prevInset() const
{
	BOOST_ASSERT(!empty());
	if (pos() == 0)
		return 0;
	if (inMathed())
		if (cell().empty())
			// FIXME: this should not happen but it does.
			// See bug 3189
			// http://bugzilla.lyx.org/show_bug.cgi?id=3189
			return 0;
		else
			return prevAtom().nucleus();
	return paragraph().isInset(pos() - 1) ? paragraph().getInset(pos() - 1) : 0;
}


InsetBase * DocIterator::realInset() const
{
	BOOST_ASSERT(inTexted());
	// if we are in a tabular, we need the cell
	if (inset().lyxCode() == InsetBase::TABULAR_CODE) {
		InsetTabular & tabular = static_cast<InsetTabular&>(inset());
		return tabular.cell(idx()).get();
	}
	return &inset();
}


MathAtom const & DocIterator::prevAtom() const
{
	BOOST_ASSERT(!empty());
	BOOST_ASSERT(pos() > 0);
	return cell()[pos() - 1];
}


MathAtom & DocIterator::prevAtom()
{
	BOOST_ASSERT(!empty());
	BOOST_ASSERT(pos() > 0);
	return cell()[pos() - 1];
}


MathAtom const & DocIterator::nextAtom() const
{
	BOOST_ASSERT(!empty());
	//lyxerr << "lastpos: " << lastpos() << " next atom:\n" << *this << endl;
	BOOST_ASSERT(pos() < lastpos());
	return cell()[pos()];
}


MathAtom & DocIterator::nextAtom()
{
	BOOST_ASSERT(!empty());
	//lyxerr << "lastpos: " << lastpos() << " next atom:\n" << *this << endl;
	BOOST_ASSERT(pos() < lastpos());
	return cell()[pos()];
}


LyXText * DocIterator::text()
{
	BOOST_ASSERT(!empty());
	return top().text();
}

LyXText const * DocIterator::text() const
{
	BOOST_ASSERT(!empty());
	return top().text();
}


Paragraph & DocIterator::paragraph()
{
	if (!inTexted())
		lyxerr << *this << endl;
	BOOST_ASSERT(inTexted());
	return top().paragraph();
}


Paragraph const & DocIterator::paragraph() const
{
	BOOST_ASSERT(inTexted());
	return top().paragraph();
}


pit_type DocIterator::lastpit() const
{
	return inMathed() ? 0 : text()->paragraphs().size() - 1;
}


pos_type DocIterator::lastpos() const
{
	return inMathed() ? cell().size() : paragraph().size();
}


DocIterator::idx_type DocIterator::lastidx() const
{
	return top().lastidx();
}


size_t DocIterator::nargs() const
{
	// assume 1x1 grid for main text
	return top().nargs();
}


size_t DocIterator::ncols() const
{
	// assume 1x1 grid for main text
	return top().ncols();
}


size_t DocIterator::nrows() const
{
	// assume 1x1 grid for main text
	return top().nrows();
}


DocIterator::row_type DocIterator::row() const
{
	return top().row();
}


DocIterator::col_type DocIterator::col() const
{
	return top().col();
}


MathData const & DocIterator::cell() const
{
//	BOOST_ASSERT(inMathed());
	return top().cell();
}


MathData & DocIterator::cell()
{
//	BOOST_ASSERT(inMathed());
	return top().cell();
}


LyXText * DocIterator::innerText()
{
	BOOST_ASSERT(!empty());
	// Go up until first non-0 text is hit
	// (innermost text is 0 in mathed)
	for (int i = depth() - 1; i >= 0; --i)
		if (slices_[i].text())
			return slices_[i].text();
	return 0;
}

LyXText const * DocIterator::innerText() const
{
	BOOST_ASSERT(!empty());
	// go up until first non-0 text is hit
	// (innermost text is 0 in mathed)
	for (int i = depth() - 1; i >= 0; --i)
		if (slices_[i].text())
			return slices_[i].text();
	return 0;
}


InsetBase * DocIterator::innerInsetOfType(int code) const
{
	for (int i = depth() - 1; i >= 0; --i)
		if (slices_[i].inset_->lyxCode() == code)
			return slices_[i].inset_;
	return 0;
}


void DocIterator::forwardPos(bool ignorecollapsed)
{
	//this dog bites his tail
	if (empty()) {
		push_back(CursorSlice(*inset_));
		return;
	}

	InsetBase * const nextinset = nextInset();
	// jump over collapsables if they are collapsed
	// FIXME: the check for asInsetMath() shouldn't be necessary
	// but math insets do not return a sensible editable() state yet.
	if (ignorecollapsed && nextinset && (!nextinset->asInsetMath()
	    && nextinset->editable() != InsetBase::HIGHLY_EDITABLE)) {
		++top().pos();
		return;
	}

	CursorSlice & tip = top();
	//lyxerr << "XXX\n" << *this << endl;

	// this is used twice and shows up in the profiler!
	pos_type const lastp = lastpos();

	// move into an inset to the right if possible
	InsetBase * n = 0;

	if (tip.pos() != lastp) {
		// this is impossible for pos() == size()
		if (inMathed()) {
			n = (tip.cell().begin() + tip.pos())->nucleus();
		} else {
			if (paragraph().isInset(tip.pos()))
				n = paragraph().getInset(tip.pos());
		}
	}

	if (n && n->isActive()) {
		//lyxerr << "... descend" << endl;
		push_back(CursorSlice(*n));
		return;
	}

	// otherwise move on one position if possible
	if (tip.pos() < lastp) {
		//lyxerr << "... next pos" << endl;
		++tip.pos();
		return;
	}
	//lyxerr << "... no next pos" << endl;

	// otherwise move on one paragraph if possible
	if (tip.pit() < lastpit()) {
		//lyxerr << "... next par" << endl;
		++tip.pit();
		tip.pos() = 0;
		return;
	}
	//lyxerr << "... no next pit" << endl;

	// otherwise try to move on one cell if possible
	if (tip.idx() < lastidx()) {
		//lyxerr << "... next idx" << endl;
		++tip.idx();
		tip.pit() = 0;
		tip.pos() = 0;
		return;
	}
	//lyxerr << "... no next idx" << endl;

	// otherwise leave inset and jump over inset as a whole
	pop_back();
	// 'top' is invalid now...
	if (!empty())
		++top().pos();
}


void DocIterator::forwardPosNoDescend()
{
	CursorSlice & tip = top();
	pos_type const lastp = lastpos();

	//  move on one position if possible
	if (tip.pos() < lastp) {
		//lyxerr << "... next pos" << endl;
		++tip.pos();
		return;
	}
	//lyxerr << "... no next pos" << endl;

	// otherwise move on one paragraph if possible
	if (tip.pit() < lastpit()) {
		//lyxerr << "... next par" << endl;
		++tip.pit();
		tip.pos() = 0;
		return;
	}
	//lyxerr << "... no next pit" << endl;

	// otherwise try to move on one cell if possible
	if (tip.idx() < lastidx()) {
		//lyxerr << "... next idx" << endl;
		++tip.idx();
		tip.pit() = 0;
		tip.pos() = 0;
		return;
	}
	//lyxerr << "... no next idx" << endl;

	// otherwise leave inset and jump over inset as a whole
	pop_back();
	// 'top' is invalid now...
	if (!empty())
		++top().pos();
}


void DocIterator::forwardPar()
{
	forwardPos();

	while (!empty() && (!inTexted() || pos() != 0)) {
		if (inTexted()) {
			pos_type const lastp = lastpos();
			Paragraph const & par = paragraph();
			pos_type & pos = top().pos();
			if (par.insetlist.empty())
				pos = lastp;
			else
				while (pos < lastp && !par.isInset(pos))
					++pos;
		}
		forwardPos();
	}
}


void DocIterator::forwardChar()
{
	forwardPos();
	while (!empty() && pos() == lastpos())
		forwardPos();
}


void DocIterator::forwardInset()
{
	forwardPos();

	while (!empty() && !nextInset()) {
		if (inTexted()) {
			pos_type const lastp = lastpos();
			Paragraph const & par = paragraph();
			pos_type & pos = top().pos();
			while (pos < lastp && !par.isInset(pos))
				++pos;
			if (pos < lastp)
				break;
		}
		forwardPos();
	}
}


void DocIterator::backwardChar()
{
	backwardPos();
	while (!empty() && pos() == lastpos())
		backwardPos();
}


void DocIterator::backwardPos()
{
	//this dog bites his tail
	if (empty()) {
		push_back(CursorSlice(*inset_));
		top().idx() = lastidx();
		top().pit() = lastpit();
		top().pos() = lastpos();
		return;
	}

	CursorSlice & tip = top();

	if (tip.pos() != 0) {
		--tip.pos();
	} else if (tip.pit() != 0) {
		--tip.pit();
		tip.pos() = lastpos();
		return;
	} else if (tip.idx() != 0) {
		--tip.idx();
		tip.pit() = lastpit();
		tip.pos() = lastpos();
		return;
	} else {
		pop_back();
		return;
	}

	// move into an inset to the left if possible
	InsetBase * n = 0;

	if (inMathed()) {
		n = (tip.cell().begin() + tip.pos())->nucleus();
	} else {
		if (paragraph().isInset(tip.pos()))
			n = paragraph().getInset(tip.pos());
	}

	if (n && n->isActive()) {
		push_back(CursorSlice(*n));
		top().idx() = lastidx();
		top().pit() = lastpit();
		top().pos() = lastpos();
	}
}


bool DocIterator::hasPart(DocIterator const & it) const
{
	// it can't be a part if it is larger
	if (it.depth() > depth())
		return false;

	// as inset adresses are the 'last' level
	return &it.top().inset() == &slices_[it.depth() - 1].inset();
}


void DocIterator::updateInsets(InsetBase * inset)
{
	// this function re-creates the cache of inset pointers.
	// code taken in part from StableDocIterator::asDocIterator.
	//lyxerr << "converting:\n" << *this << endl;
	DocIterator dit = DocIterator(*inset);
	size_t const n = slices_.size();
	for (size_t i = 0 ; i < n; ++i) {
		BOOST_ASSERT(inset);
		dit.push_back(slices_[i]);
		dit.top().inset_ = inset;
		if (i + 1 != n)
			inset = dit.nextInset();
	}
	//lyxerr << "converted:\n" << *this << endl;
	operator=(dit);
}


std::ostream & operator<<(std::ostream & os, DocIterator const & dit)
{
	for (size_t i = 0, n = dit.depth(); i != n; ++i)
		os << " " << dit[i] << "\n";
	return os;
}



///////////////////////////////////////////////////////

StableDocIterator::StableDocIterator(DocIterator const & dit)
{
	data_ = dit.internalData();
	for (size_t i = 0, n = data_.size(); i != n; ++i)
		data_[i].inset_ = 0;
}


DocIterator StableDocIterator::asDocIterator(InsetBase * inset) const
{
	// this function re-creates the cache of inset pointers
	//lyxerr << "converting:\n" << *this << endl;
	DocIterator dit = DocIterator(*inset);
	for (size_t i = 0, n = data_.size(); i != n; ++i) {
		if (inset == 0) {
			// FIXME
			lyxerr << BOOST_CURRENT_FUNCTION
			       << " Should not happen, but does e.g. after C-n C-l C-z S-C-z"
				<< '\n' << "dit: " << dit << '\n'
				<< " lastpos: " << dit.lastpos() << endl;
			//break;
			BOOST_ASSERT(false);
		}
		dit.push_back(data_[i]);
		dit.top().inset_ = inset;
		if (i + 1 != n)
			inset = dit.nextInset();
	}
	//lyxerr << "convert:\n" << *this << " to:\n" << dit << endl;
	return dit;
}


std::ostream & operator<<(std::ostream & os, StableDocIterator const & dit)
{
	for (size_t i = 0, n = dit.data_.size(); i != n; ++i)
		os << " " << dit.data_[i] << "\n";
	return os;
}


bool operator==(StableDocIterator const & dit1, StableDocIterator const & dit2)
{
	return dit1.data_ == dit2.data_;
}


} // namespace lyx
