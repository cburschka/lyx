
#include "dociterator.h"

#include "BufferView.h"
#include "debug.h"
#include "lyxtext.h"
#include "lyxrow.h"
#include "paragraph.h"

#include "mathed/math_data.h"
#include "mathed/math_inset.h"

#include <boost/assert.hpp>



std::ostream & operator<<(std::ostream & os, DocumentIterator const & cur);


DocumentIterator::DocumentIterator()
	: bv_(0)
{}


DocumentIterator::DocumentIterator(BufferView & bv)
	: std::vector<CursorSlice>(1), bv_(&bv)
{}


InsetBase * DocumentIterator::nextInset()
{
	if (pos() == lastpos())
		return 0;
	if (inMathed()) 
		return nextAtom().nucleus();
	return paragraph().isInset(pos()) ? paragraph().getInset(pos()) : 0;
}


InsetBase * DocumentIterator::prevInset()
{
	if (pos() == 0)
		return 0;
	if (inMathed()) 
		return prevAtom().nucleus();
	return paragraph().isInset(pos() - 1) ? paragraph().getInset(pos() - 1) : 0;
}


InsetBase const * DocumentIterator::prevInset() const
{
	if (pos() == 0)
		return 0;
	if (inMathed()) 
		return prevAtom().nucleus();
	return paragraph().isInset(pos() - 1) ? paragraph().getInset(pos() - 1) : 0;
}


MathAtom const & DocumentIterator::prevAtom() const
{
	BOOST_ASSERT(pos() > 0);
	return cell()[pos() - 1];
}


MathAtom & DocumentIterator::prevAtom()
{
	BOOST_ASSERT(pos() > 0);
	return cell()[pos() - 1];
}


MathAtom const & DocumentIterator::nextAtom() const
{
	BOOST_ASSERT(pos() < lastpos());
	return cell()[pos()];
}


MathAtom & DocumentIterator::nextAtom()
{
	BOOST_ASSERT(pos() < lastpos());
	return cell()[pos()];
}


LyXText * DocumentIterator::text() const
{
	return size() > 1 ? top().text() : bv().text();
}


Paragraph & DocumentIterator::paragraph()
{
	BOOST_ASSERT(inTexted());
	return size() > 1 ? top().paragraph() : *bv().text()->getPar(par());
}


Paragraph const & DocumentIterator::paragraph() const
{
	BOOST_ASSERT(inTexted());
	return size() > 1 ? top().paragraph() : *bv().text()->getPar(par());
}


Row & DocumentIterator::textRow()
{
	return *paragraph().getRow(pos());
}


Row const & DocumentIterator::textRow() const
{
	return *paragraph().getRow(pos());
}


DocumentIterator::par_type DocumentIterator::lastpar() const
{
	return inMathed() ? 0 : text()->paragraphs().size() - 1;
}


DocumentIterator::pos_type DocumentIterator::lastpos() const
{
	return inMathed() ? cell().size() : paragraph().size();
}


DocumentIterator::row_type DocumentIterator::crow() const
{
	return paragraph().row(pos());
}


DocumentIterator::row_type DocumentIterator::lastcrow() const
{
	return paragraph().rows.size();
}


DocumentIterator::idx_type DocumentIterator::lastidx() const
{
	return size() > 1 ? top().lastidx() : 0;
}


size_t DocumentIterator::nargs() const
{
	// assume 1x1 grid for main text
	return size() > 1 ? top().nargs() : 1;
}


size_t DocumentIterator::ncols() const
{
	// assume 1x1 grid for main text
	return size() > 1 ? top().ncols() : 1;
}


size_t DocumentIterator::nrows() const
{
	// assume 1x1 grid for main text
	return size() > 1 ? top().nrows() : 1;
}


DocumentIterator::row_type DocumentIterator::row() const
{
	return size() > 1 ? top().row() : 0;
}


DocumentIterator::col_type DocumentIterator::col() const
{
	return size() > 1 ? top().col() : 0;
}


MathArray const & DocumentIterator::cell() const
{
	BOOST_ASSERT(size() > 1);
	return top().cell();
}


MathArray & DocumentIterator::cell()
{
	BOOST_ASSERT(size() > 1);
	return top().cell();
}


bool DocumentIterator::inMathed() const
{
	return size() > 1 && inset()->inMathed();
}


bool DocumentIterator::inTexted() const
{
	return !inMathed();
}


LyXText * DocumentIterator::innerText() const
{
	BOOST_ASSERT(!empty());
	if (size() > 1) {
		// go up until first non-0 text is hit
		// (innermost text is 0 in mathed)
		for (int i = size() - 1; i >= 1; --i)
			if (operator[](i).text())
				return operator[](i).text();
	}
	return bv().text();
}


CursorSlice const & DocumentIterator::innerTextSlice() const
{
	BOOST_ASSERT(!empty());
	if (size() > 1) {
		// go up until first non-0 text is hit
		// (innermost text is 0 in mathed)
		for (int i = size() - 1; i >= 1; --i)
			if (operator[](i).text())
				return operator[](i);
	}
	return operator[](0);
}


InsetBase * DocumentIterator::innerInsetOfType(int code) const
{
	for (int i = size() - 1; i >= 1; --i)
		if (operator[](i).inset_->lyxCode() == code)
			return operator[](i).inset_;
	return 0;
}


void DocumentIterator::forwardPos()
{
	CursorSlice & top = back();
	//lyxerr << "XXX\n" << *this << std::endl;

	// move into an inset to the right if possible
	InsetBase * n = 0;
	if (top.pos() != lastpos()) {
		// this is impossible for pos() == size()
		if (inMathed()) {
			n = (top.cell().begin() + top.pos())->nucleus();
		} else {
			if (paragraph().isInset(top.pos()))
				n = paragraph().getInset(top.pos());
		}
	}

	if (n && n->isActive()) {
		//lyxerr << "... descend" << std::endl;
		push_back(CursorSlice(n));
		return;
	}

	// otherwise move on one cell back if possible
	if (top.pos() < lastpos()) {
		//lyxerr << "... next pos" << std::endl;
		++top.pos();
		return;
	}
	//lyxerr << "... no next pos" << std::endl;

	// otherwise move on one cell back if possible
	if (top.par() < lastpar()) {
		//lyxerr << "... next par" << std::endl;
		++top.par();
		top.pos() = 0;
		return;
	}
	//lyxerr << "... no next par" << std::endl;

	// otherwise try to move on one cell if possible
	// [stupid hack for necessary for MathScriptInset]
	while (top.idx() < lastidx()) {
		//lyxerr << "... next idx" << std::endl;
		++top.idx();
		top.par() = 0;
		top.pos() = 0;
		if (top.inset() && top.inset()->validCell(top.idx())) {
			//lyxerr << "     ... ok" << std::endl;
			return;
		}
	}
	//lyxerr << "... no next idx" << std::endl;

	// otherwise leave inset an jump over inset as a whole
	pop_back();
	// 'top' is invalid now...
	if (size())
		++back().pos_;
	//else
	//	lyxerr << "... no slice left" << std::endl;
}


void DocumentIterator::forwardPar()
{
	CursorSlice & top = back();
	lyxerr << "XXX " << *this << std::endl;

	// move into an inset to the right if possible
	InsetBase * n = 0;
	if (top.pos() != lastpos()) {
		// this is impossible for pos() == size()
		if (inMathed()) {
			n = (top.cell().begin() + top.pos())->nucleus();
		} else {
			if (paragraph().isInset(top.pos()))
				n = paragraph().getInset(top.pos());
		}
	}

	if (n && n->isActive()) {
		lyxerr << "... descend" << std::endl;
		push_back(CursorSlice(n));
		return;
	}

	// otherwise move on one cell back if possible
	if (top.pos() < lastpos()) {
		lyxerr << "... next pos" << std::endl;
		++top.pos();
		return;
	}

	// otherwise move on one cell back if possible
	if (top.par() < lastpar()) {
		lyxerr << "... next par" << std::endl;
		++top.par();
		top.pos() = 0;
		return;
	}

	// otherwise try to move on one cell if possible
	// [stupid hack for necessary for MathScriptInset]
	while (top.idx() < top.lastidx()) {
		lyxerr << "... next idx" 
			<< " was: " << top.idx() << " max: " << top.lastidx() << std::endl;
		++top.idx();
		top.par() = 0;
		top.pos() = 0;
		if (top.inset() && top.inset()->validCell(top.idx())) {
			lyxerr << "     ... ok" << std::endl;
			return;
		}
	}

	// otherwise leave inset an jump over inset as a whole
	pop_back();
	// 'top' is invalid now...
	if (size())
		++back().pos_;
}


DocumentIterator bufferBegin(BufferView & bv)
{
	return DocumentIterator(bv);
}


DocumentIterator bufferEnd()
{
	return DocumentIterator();
}


DocumentIterator insetBegin(BufferView & bv, InsetBase * p)
{
	DocumentIterator it(bv);
	it.back() = CursorSlice(p);
	return it;
}


DocumentIterator insetEnd()
{
	return DocumentIterator();
}


std::ostream & operator<<(std::ostream & os, DocumentIterator const & cur)
{
	for (size_t i = 0, n = cur.size(); i != n; ++i)
		os << " " << cur.operator[](i) << "\n";
	return os;
}
