/**
 * \file DocIterator.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */


#include <config.h>

#include "DocIterator.h"

#include "debug.h"
#include "Text.h"
#include "Paragraph.h"

#include "mathed/MathData.h"
#include "mathed/InsetMath.h"

#include "insets/InsetTabular.h"

#include <boost/assert.hpp>
#include <boost/current_function.hpp>

using std::endl;


namespace lyx {


// We could be able to get rid of this if only every BufferView were
// associated to a buffer on construction.
DocIterator::DocIterator()
	: boundary_(false), inset_(0)
{}


DocIterator::DocIterator(Inset & inset)
	: boundary_(false), inset_(&inset)
{}


DocIterator doc_iterator_begin(Inset & inset)
{
	DocIterator dit(inset);
	dit.forwardPos();
	return dit;
}


DocIterator doc_iterator_end(Inset & inset)
{
	return DocIterator(inset);
}


Inset * DocIterator::nextInset()
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


Inset * DocIterator::prevInset()
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


Inset const * DocIterator::prevInset() const
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


Inset * DocIterator::realInset() const
{
	BOOST_ASSERT(inTexted());
	// if we are in a tabular, we need the cell
	if (inset().lyxCode() == Inset::TABULAR_CODE) {
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


Text * DocIterator::text()
{
	BOOST_ASSERT(!empty());
	return top().text();
}

Text const * DocIterator::text() const
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


Paragraph const & DocIterator::innerParagraph() const
{
	BOOST_ASSERT(!empty());
	// go up until first non-0 text is hit
	// (innermost text is 0 in mathed)
	for (int i = depth() - 1; i >= 0; --i)
		if (slices_[i].text())
			return slices_[i].paragraph();

	// This case is in principe not possible. We _must_
	// be inside a Paragraph.
	BOOST_ASSERT(false);
	return paragraph();
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


Text * DocIterator::innerText()
{
	BOOST_ASSERT(!empty());
	// Go up until first non-0 text is hit
	// (innermost text is 0 in mathed)
	for (int i = depth() - 1; i >= 0; --i)
		if (slices_[i].text())
			return slices_[i].text();
	return 0;
}

Text const * DocIterator::innerText() const
{
	BOOST_ASSERT(!empty());
	// go up until first non-0 text is hit
	// (innermost text is 0 in mathed)
	for (int i = depth() - 1; i >= 0; --i)
		if (slices_[i].text())
			return slices_[i].text();
	return 0;
}


Inset * DocIterator::innerInsetOfType(int code) const
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

	Inset * const nextinset = nextInset();
	// jump over collapsables if they are collapsed
	// FIXME: the check for asInsetMath() shouldn't be necessary
	// but math insets do not return a sensible editable() state yet.
	if (ignorecollapsed && nextinset && (!nextinset->asInsetMath()
	    && nextinset->editable() != Inset::HIGHLY_EDITABLE)) {
		++top().pos();
		return;
	}

	CursorSlice & tip = top();
	//lyxerr << "XXX\n" << *this << endl;

	// this is used twice and shows up in the profiler!
	pos_type const lastp = lastpos();

	// move into an inset to the right if possible
	Inset * n = 0;

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

	if (!tip.at_end()) {
		tip.forwardPos();
		return;
	}
	// otherwise leave inset and jump over inset as a whole
	pop_back();
	// 'tip' is invalid now...
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


void DocIterator::forwardIdx()
{
	CursorSlice & tip = top();

	//prevent endless loops
	BOOST_ASSERT(tip.idx() < tip.nargs());
	++tip.idx();
	tip.pit() = 0;
	tip.pos() = 0;
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

	if (top().at_begin()) {
		pop_back();
		return;
	}

	top().backwardPos();

	// move into an inset to the left if possible
	Inset * n = 0;

	if (inMathed()) {
		n = (top().cell().begin() + top().pos())->nucleus();
	} else {
		if (paragraph().isInset(top().pos()))
			n = paragraph().getInset(top().pos());
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


void DocIterator::updateInsets(Inset * inset)
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


bool DocIterator::fixIfBroken()
{
	// Go through the slice stack from the bottom. 
	// Check that all coordinates (idx, pit, pos) are correct and
	// that the inset is the one which is claimed to be there
	Inset * inset = &slices_[0].inset();
	size_t i = 0;
	size_t n = slices_.size();
	for (; i != n; ++i) {
		CursorSlice & cs = slices_[i];
		if (&cs.inset() != inset) {
			// the whole slice is wrong, chop off this as well
			--i;
			LYXERR(Debug::DEBUG) << "fixIfBroken(): inset changed" << endl;
			break;
		} else if (cs.idx() > cs.lastidx()) {
			cs.idx() = cs.lastidx();
			cs.pit() = cs.lastpit();
			cs.pos() = cs.lastpos();
			LYXERR(Debug::DEBUG) << "fixIfBroken(): idx fixed" << endl;
			break;
		} else if (cs.pit() > cs.lastpit()) {
			cs.pit() = cs.lastpit();
			cs.pos() = cs.lastpos();
			LYXERR(Debug::DEBUG) << "fixIfBroken(): pit fixed" << endl;
			break;
		} else if (cs.pos() > cs.lastpos()) {
			cs.pos() = cs.lastpos();
			LYXERR(Debug::DEBUG) << "fixIfBroken(): pos fixed" << endl;
			break;
		} else if (i != n - 1 && cs.pos() != cs.lastpos()) {
			// get inset which is supposed to be in the next slice
			if (cs.inset().inMathed())
				inset = (cs.cell().begin() + cs.pos())->nucleus();
			else if (cs.paragraph().isInset(cs.pos()))
				inset = cs.paragraph().getInset(cs.pos());
			else {
				// there are slices left, so there must be another inset
				break;
			}
		}
	}

	// Did we make it through the whole slice stack? Otherwise there
	// was a problem at slice i, and we have to chop off above
	if (i < n) {
		LYXERR(Debug::DEBUG) << "fixIfBroken(): cursor chopped at " << i << endl;
		resize(i + 1);
		return true;
	} else
		return false;
}


std::ostream & operator<<(std::ostream & os, DocIterator const & dit)
{
	for (size_t i = 0, n = dit.depth(); i != n; ++i)
		os << " " << dit[i] << "\n";
	return os;
}


bool operator<(DocIterator const & p, DocIterator const & q)
{
	size_t depth = std::min(p.depth(), q.depth());
	for (size_t i = 0 ; i < depth ; ++i) {
		if (p[i] != q[i])
			return p[i] < q[i];
	}
	return p.depth() < q.depth();
}


bool operator>(DocIterator const & p, DocIterator const & q)
{
	return q < p;
}


bool operator<=(DocIterator const & p, DocIterator const & q)
{
	return !(q < p);
}


///////////////////////////////////////////////////////

StableDocIterator::StableDocIterator(DocIterator const & dit)
{
	data_ = dit.internalData();
	for (size_t i = 0, n = data_.size(); i != n; ++i)
		data_[i].inset_ = 0;
}


DocIterator StableDocIterator::asDocIterator(Inset * inset) const
{
	// this function re-creates the cache of inset pointers
	//lyxerr << "converting:\n" << *this << endl;
	DocIterator dit = DocIterator(*inset);
	for (size_t i = 0, n = data_.size(); i != n; ++i) {
		if (inset == 0) {
			// FIXME
			lyxerr << BOOST_CURRENT_FUNCTION
			       << " Should not happen, but does e.g. after C-n C-l C-z S-C-z\n"
				   << " or when a Buffer has been concurently edited by two views"
				<< '\n' << "dit: " << dit << '\n'
				<< " lastpos: " << dit.lastpos() << endl;
			dit.fixIfBroken();
			break;
		}
		dit.push_back(data_[i]);
		dit.top().inset_ = inset;
		if (dit.fixIfBroken())
			break;
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
