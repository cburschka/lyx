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

#include "Buffer.h"
#include "InsetList.h"
#include "Paragraph.h"
#include "LyXRC.h"
#include "Text.h"

#include "mathed/MathData.h"
#include "mathed/InsetMath.h"
#include "mathed/InsetMathHull.h"

#include "insets/InsetTabular.h"

#include "support/debug.h"
#include "support/ExceptionMessage.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

#include <ostream>

using namespace std;
using namespace lyx::support;

namespace lyx {


DocIterator::DocIterator()
	: boundary_(false), inset_(0), buffer_(0)
{}


// We could be able to get rid of this if only every BufferView were
// associated to a buffer on construction.
DocIterator::DocIterator(Buffer * buf)
	: boundary_(false), inset_(0), buffer_(buf)
{}


DocIterator::DocIterator(Buffer * buf, Inset * inset)
	: boundary_(false), inset_(inset), buffer_(buf)
{}


DocIterator doc_iterator_begin(const Buffer * buf0, const Inset * inset0)
{
	Buffer * buf = const_cast<Buffer *>(buf0);	
	Inset * inset = const_cast<Inset *>(inset0);
	DocIterator dit(buf, inset ? inset : &buf->inset());
	dit.forwardPos();
	return dit;
}


DocIterator doc_iterator_end(const Buffer * buf0, const Inset * inset0)
{
	Buffer * buf = const_cast<Buffer *>(buf0);	
	Inset * inset = const_cast<Inset *>(inset0);
	return DocIterator(buf, inset ? inset : &buf->inset());
}


DocIterator DocIterator::clone(Buffer * buffer) const
{
	LASSERT(buffer->isClone(), return DocIterator());
	Inset * inset = &buffer->inset();
	DocIterator dit(buffer);
	size_t const n = slices_.size();
	for (size_t i = 0 ; i != n; ++i) {
		LBUFERR(inset);
		dit.push_back(slices_[i]);
		dit.top().inset_ = inset;
		if (i + 1 != n)
			inset = dit.nextInset();
	}
	return dit;
}


bool DocIterator::inRegexped() const
{
	InsetMath * im = inset().asInsetMath();
	if (!im) 
		return false;
	InsetMathHull * hull = im->asHullInset();
	return hull && hull->getType() == hullRegexp;
}


LyXErr & operator<<(LyXErr & os, DocIterator const & it)
{
	os.stream() << it;
	return os;
}


Inset * DocIterator::nextInset() const
{
	LASSERT(!empty(), return 0);
	if (pos() == lastpos())
		return 0;
	if (pos() > lastpos()) {
		LYXERR0("Should not happen, but it does: pos() = "
			<< pos() << ", lastpos() = " << lastpos());
		return 0;
	}
	if (inMathed())
		return nextAtom().nucleus();
	return paragraph().getInset(pos());
}


Inset * DocIterator::prevInset() const
{
	LASSERT(!empty(), return 0);
	if (pos() == 0)
		return 0;
	if (inMathed()) {
		if (cell().empty())
			// FIXME: this should not happen but it does.
			// See bug 3189
			// http://www.lyx.org/trac/ticket/3189
			return 0;
		else
			return prevAtom().nucleus();
	}
	return paragraph().getInset(pos() - 1);
}


Inset * DocIterator::realInset() const
{
	LASSERT(inTexted(), return 0);
	// if we are in a tabular, we need the cell
	if (inset().lyxCode() == TABULAR_CODE) {
		InsetTabular * tabular = inset().asInsetTabular();
		return tabular->cell(idx()).get();
	}
	return &inset();
}


MathAtom & DocIterator::prevAtom() const
{
	LASSERT(!empty(), /**/);
	LASSERT(pos() > 0, /**/);
	return cell()[pos() - 1];
}


MathAtom & DocIterator::nextAtom() const
{
	LASSERT(!empty(), /**/);
	//lyxerr << "lastpos: " << lastpos() << " next atom:\n" << *this << endl;
	LASSERT(pos() < lastpos(), /**/);
	return cell()[pos()];
}


Text * DocIterator::text() const
{
	LASSERT(!empty(), return 0);
	return top().text();
}


Paragraph & DocIterator::paragraph() const
{
	if (!inTexted()) {
		LYXERR0(*this);
		LBUFERR(false);
	}
	return top().paragraph();
}


Paragraph & DocIterator::innerParagraph() const
{
	LBUFERR(!empty());
	return innerTextSlice().paragraph();
}


FontSpan DocIterator::locateWord(word_location const loc) const
{
	FontSpan f = FontSpan();

	if (!top().text()->empty()) {
		f.first = pos();
		top().paragraph().locateWord(f.first, f.last, loc);
	}
	return f;
}

	
CursorSlice const & DocIterator::innerTextSlice() const
{
	LBUFERR(!empty());
	// go up until first non-0 text is hit
	// (innermost text is 0 in mathed)
	for (int i = depth() - 1; i >= 0; --i)
		if (slices_[i].text())
			return slices_[i];

	// This case is in principle not possible. We _must_
	// be inside a Text.
	LBUFERR(false);
	// Squash warning
	static const CursorSlice c;
	return c;
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


MathData & DocIterator::cell() const
{
//	LASSERT(inMathed(), /**/);
	return top().cell();
}


Text * DocIterator::innerText() const
{
	LASSERT(!empty(), return 0);
	return innerTextSlice().text();
}


Inset * DocIterator::innerInsetOfType(int code) const
{
	for (int i = depth() - 1; i >= 0; --i)
		if (slices_[i].inset_->lyxCode() == code)
			return slices_[i].inset_;
	return 0;
}


// This duplicates code above, but is in the critical path.
// So please think twice before adding stuff
void DocIterator::forwardPos()
{
	// this dog bites his tail
	if (empty()) {
		push_back(CursorSlice(*inset_));
		return;
	}

	CursorSlice & tip = top();
	//lyxerr << "XXX\n" << *this << endl;

	// not at cell/paragraph end?
	if (tip.pos() != tip.lastpos()) {
		// move into an inset to the right if possible
		Inset * n = 0;
		if (inMathed())
			n = (tip.cell().begin() + tip.pos())->nucleus();
		else
			n = paragraph().getInset(tip.pos());
		if (n && n->isActive()) {
			//lyxerr << "... descend" << endl;
			push_back(CursorSlice(*n));
			return;
		}
	}

	// jump to the next cell/paragraph if possible
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


void DocIterator::forwardPosIgnoreCollapsed()
{
	Inset * const nextinset = nextInset();
	// FIXME: the check for asInsetMath() shouldn't be necessary
	// but math insets do not return a sensible editable() state yet.
	if (nextinset && !nextinset->asInsetMath()
	    && !nextinset->editable()) {
		++top().pos();
		return;
	}
	forwardPos();
}


void DocIterator::forwardPar()
{
	forwardPos();

	while (!empty() && (!inTexted() || pos() != 0)) {
		if (inTexted()) {
			pos_type const lastp = lastpos();
			Paragraph const & par = paragraph();
			pos_type & pos = top().pos();
			if (par.insetList().empty())
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

	// at inset beginning?
	if (top().at_begin()) {
		pop_back();
		return;
	}

	top().backwardPos();

	// entered another cell/paragraph from the right?
	if (top().pos() == top().lastpos())
		return;

	// move into an inset to the left if possible
	Inset * n = 0;
	if (inMathed())
		n = (top().cell().begin() + top().pos())->nucleus();
	else
		n = paragraph().getInset(top().pos());
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
	//lyxerr << "converting:\n" << *this << endl;
	DocIterator dit = *this;
	size_t const n = slices_.size();
	slices_.resize(0);
	for (size_t i = 0 ; i < n; ++i) {
		LBUFERR(inset);
		push_back(dit[i]);
		top().inset_ = inset;
		if (i + 1 != n)
			inset = nextInset();
	}
	//lyxerr << "converted:\n" << *this << endl;
}


bool DocIterator::fixIfBroken()
{
	if (empty())
		return false;

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
			LYXERR(Debug::DEBUG, "fixIfBroken(): inset changed");
			break;
		} else if (cs.idx() > cs.lastidx()) {
			cs.idx() = cs.lastidx();
			cs.pit() = cs.lastpit();
			cs.pos() = cs.lastpos();
			LYXERR(Debug::DEBUG, "fixIfBroken(): idx fixed");
			break;
		} else if (cs.pit() > cs.lastpit()) {
			cs.pit() = cs.lastpit();
			cs.pos() = cs.lastpos();
			LYXERR(Debug::DEBUG, "fixIfBroken(): pit fixed");
			break;
		} else if (cs.pos() > cs.lastpos()) {
			cs.pos() = cs.lastpos();
			LYXERR(Debug::DEBUG, "fixIfBroken(): pos fixed");
			break;
		} else if (i != n - 1 && cs.pos() != cs.lastpos()) {
			// get inset which is supposed to be in the next slice
			if (cs.inset().inMathed())
				inset = (cs.cell().begin() + cs.pos())->nucleus();
			else if (Inset * csInset = cs.paragraph().getInset(cs.pos()))
				inset = csInset;
			else {
				// there are slices left, so there must be another inset
				break;
			}
		}
	}

	// Did we make it through the whole slice stack? Otherwise there
	// was a problem at slice i, and we have to chop off above
	if (i < n) {
		LYXERR(Debug::DEBUG, "fixIfBroken(): cursor chopped at " << i);
		resize(i + 1);
		return true;
	} else
		return false;
}


void DocIterator::sanitize()
{
	// keep a copy of the slices
	vector<CursorSlice> const sl = slices_;
	slices_.clear();
	if (buffer_)
		inset_ = &buffer_->inset();
	Inset * inset = inset_;
	// re-add the slices one by one, and adjust the inset pointer.
	for (size_t i = 0, n = sl.size(); i != n; ++i) {
		if (inset == 0) {
			// FIXME
			LYXERR0(" Should not happen, but does e.g. after "
				"C-n C-l C-z S-C-z\n"
				<< " or when a Buffer has been concurrently edited by two views"
				<< '\n' << "dit: " << *this << '\n'
				<< " lastpos: " << slices_[i].lastpos());
			fixIfBroken();
			break;
		}
		if (!inset->isActive()) {
			LYXERR0("Inset found on cursor stack is not active.");
			fixIfBroken();
			break;
		}
		push_back(sl[i]);
		top().inset_ = inset;
		if (fixIfBroken())
			break;
		if (i + 1 != n)
			inset = nextInset();
	}
}


int DocIterator::find(MathData const & cell) const
{
	for (size_t l = 0; l != slices_.size(); ++l) {
		if (slices_[l].asInsetMath() && &slices_[l].cell() == &cell)
			return l;
	}
	return -1;
}


int DocIterator::find(Inset const * inset) const 
{
	for (size_t l = 0; l != slices_.size(); ++l) {
		if (&slices_[l].inset() == inset)
			return l;
	}
	return -1;
}


void DocIterator::cutOff(int above, vector<CursorSlice> & cut)
{
	cut = vector<CursorSlice>(slices_.begin() + above + 1, slices_.end());
	slices_.resize(above + 1);
}


void DocIterator::cutOff(int above)
{
	slices_.resize(above + 1);
}


void DocIterator::append(vector<CursorSlice> const & x) 
{
	slices_.insert(slices_.end(), x.begin(), x.end());
}


void DocIterator::append(DocIterator::idx_type idx, pos_type pos) 
{
	slices_.push_back(CursorSlice());
	top().idx() = idx;
	top().pos() = pos;
}


ostream & operator<<(ostream & os, DocIterator const & dit)
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


DocIterator StableDocIterator::asDocIterator(Buffer * buf) const
{
	DocIterator dit(buf);
	dit.slices_ = data_;
	dit.sanitize();
	return dit;
}


ostream & operator<<(ostream & os, StableDocIterator const & dit)
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
