
#include "dociterator.h"

#include "debug.h"
#include "lyxtext.h"
#include "lyxrow.h"
#include "paragraph.h"

#include "mathed/math_data.h"
#include "mathed/math_inset.h"

#include <boost/assert.hpp>

using std::endl;


//we could be able to get rid of this if only every BufferView were
//associated to a buffer on construction
DocumentIterator::DocumentIterator() : inset_(0)
{}


DocumentIterator doc_iterator_begin(InsetBase & inset)
{
	DocumentIterator dit(inset);
	dit.forwardPos();
	return dit;
}


DocumentIterator doc_iterator_end(InsetBase & inset)
{
	return DocumentIterator(inset);
}


DocumentIterator::DocumentIterator(InsetBase & inset) : inset_(&inset)
{}


InsetBase * DocumentIterator::nextInset()
{
	BOOST_ASSERT(!empty());
	if (pos() == lastpos())
		return 0;
	if (inMathed()) 
		return nextAtom().nucleus();
	return paragraph().isInset(pos()) ? paragraph().getInset(pos()) : 0;
}


InsetBase * DocumentIterator::prevInset()
{
	BOOST_ASSERT(!empty());
	if (pos() == 0)
		return 0;
	if (inMathed()) 
		return prevAtom().nucleus();
	return paragraph().isInset(pos() - 1) ? paragraph().getInset(pos() - 1) : 0;
}


InsetBase const * DocumentIterator::prevInset() const
{
	BOOST_ASSERT(!empty());
	if (pos() == 0)
		return 0;
	if (inMathed()) 
		return prevAtom().nucleus();
	return paragraph().isInset(pos() - 1) ? paragraph().getInset(pos() - 1) : 0;
}


MathAtom const & DocumentIterator::prevAtom() const
{
	BOOST_ASSERT(!empty());
	BOOST_ASSERT(pos() > 0);
	return cell()[pos() - 1];
}


MathAtom & DocumentIterator::prevAtom()
{
	BOOST_ASSERT(!empty());
	BOOST_ASSERT(pos() > 0);
	return cell()[pos() - 1];
}


MathAtom const & DocumentIterator::nextAtom() const
{
	BOOST_ASSERT(!empty());
	BOOST_ASSERT(pos() < lastpos());
	return cell()[pos()];
}


MathAtom & DocumentIterator::nextAtom()
{
	BOOST_ASSERT(!empty());
	BOOST_ASSERT(pos() < lastpos());
	return cell()[pos()];
}


LyXText * DocumentIterator::text() const
{
	BOOST_ASSERT(!empty());
	return top().text();
}


Paragraph & DocumentIterator::paragraph()
{
	BOOST_ASSERT(inTexted());
	return top().paragraph();
}


Paragraph const & DocumentIterator::paragraph() const
{
	BOOST_ASSERT(inTexted());
	return top().paragraph();
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
	return top().lastidx();
}


size_t DocumentIterator::nargs() const
{
	// assume 1x1 grid for main text
	return top().nargs();
}


size_t DocumentIterator::ncols() const
{
	// assume 1x1 grid for main text
	return top().ncols();
}


size_t DocumentIterator::nrows() const
{
	// assume 1x1 grid for main text
	return top().nrows();
}


DocumentIterator::row_type DocumentIterator::row() const
{
	return top().row();
}


DocumentIterator::col_type DocumentIterator::col() const
{
	return top().col();
}


MathArray const & DocumentIterator::cell() const
{
	BOOST_ASSERT(inMathed());
	return top().cell();
}


MathArray & DocumentIterator::cell()
{
	BOOST_ASSERT(inMathed());
	return top().cell();
}


bool DocumentIterator::inMathed() const
{
	return !empty() && inset().inMathed();
}


bool DocumentIterator::inTexted() const
{
	return !empty() && !inset().inMathed();
}


LyXText * DocumentIterator::innerText() const
{
	BOOST_ASSERT(!empty());
	// go up until first non-0 text is hit
	// (innermost text is 0 in mathed)
	for (int i = size() - 1; i >= 0; --i)
		if (operator[](i).text())
			return operator[](i).text();
	return 0;
}


InsetBase * DocumentIterator::innerInsetOfType(int code) const
{
	for (int i = size() - 1; i >= 0; --i)
		if (operator[](i).inset_->lyxCode() == code)
			return operator[](i).inset_;
	return 0;
}


void DocumentIterator::forwardPos()
{
	//this dog bites his tail
	if (empty()) {
		push_back(CursorSlice(*inset_));
		return;
	}

	CursorSlice & top = back();
	//lyxerr << "XXX\n" << *this << endl;

	// this is used twice and shows up in the profiler!
	pos_type const lastp = lastpos();

	// move into an inset to the right if possible
	InsetBase * n = 0;

	if (top.pos() != lastp) {
		// this is impossible for pos() == size()
		if (inMathed()) {
			n = (top.cell().begin() + top.pos())->nucleus();
		} else {
			if (paragraph().isInset(top.pos()))
				n = paragraph().getInset(top.pos());
		}
	}

	if (n && n->isActive()) {
		//lyxerr << "... descend" << endl;
		push_back(CursorSlice(*n));
		return;
	}

	// otherwise move on one position if possible
	if (top.pos() < lastp) {
		//lyxerr << "... next pos" << endl;
		++top.pos();
		return;
	}
	//lyxerr << "... no next pos" << endl;

	// otherwise move on one paragraph if possible
	if (top.par() < lastpar()) {
		//lyxerr << "... next par" << endl;
		++top.par();
		top.pos() = 0;
		return;
	}
	//lyxerr << "... no next par" << endl;

	// otherwise try to move on one cell if possible
	if (top.idx() < lastidx()) {
		//lyxerr << "... next idx" << endl;
		++top.idx();
		top.par() = 0;
		top.pos() = 0;
		return;
	}
	//lyxerr << "... no next idx" << endl;

	// otherwise leave inset and jump over inset as a whole
	pop_back();
	// 'top' is invalid now...
	if (size())
		++back().pos();
}


void DocumentIterator::forwardPar()
{
	forwardPos();
	while (!empty() && (!inTexted() || pos() != 0))
		forwardPos();
}


void DocumentIterator::forwardChar()
{
	forwardPos();
	while (size() != 0 && pos() == lastpos())
		forwardPos(); 
}


void DocumentIterator::forwardInset()
{
	forwardPos(); 
	while (size() != 0 && (pos() == lastpos() || nextInset() == 0))
		forwardPos();
}


void DocumentIterator::backwardChar()
{
	backwardPos();
	while (size() != 0 && pos() == lastpos())
		backwardPos(); 
}


void DocumentIterator::backwardPos()
{
	//this dog bites his tail
	if (empty()) {
		push_back(CursorSlice(*inset_));
		back().idx() = lastidx();
		back().par() = lastpar();
		back().pos() = lastpos();
		return;
	}

	CursorSlice & top = back();

	if (top.pos() != 0) {
		--top.pos();
	} else if (top.par() != 0) {
		--top.par();
		top.pos() = lastpos();
		return;
	} else if (top.idx() != 0) {
		--top.idx();
		top.par() = lastpar();
		top.pos() = lastpos();
		return;
	} else {
		pop_back();
		return;
	}

	// move into an inset to the left if possible
	InsetBase * n = 0;
	
	if (inMathed()) {
		n = (top.cell().begin() + top.pos())->nucleus();
	} else {
		if (paragraph().isInset(top.pos()))
			n = paragraph().getInset(top.pos());
	}

	if (n && n->isActive()) {
		push_back(CursorSlice(*n));
		back().idx() = lastidx();
		back().par() = lastpar();
		back().pos() = lastpos();
	}
}


std::ostream & operator<<(std::ostream & os, DocumentIterator const & dit)
{
	for (size_t i = 0, n = dit.size(); i != n; ++i)
		os << " " << dit.operator[](i) << "\n";
	return os;
}



///////////////////////////////////////////////////////

StableDocumentIterator::StableDocumentIterator(const DocumentIterator & dit)
{
	data_ = dit;
	for (size_t i = 0, n = data_.size(); i != n; ++i)
		data_[i].inset_ = 0;
}


DocumentIterator
StableDocumentIterator::asDocumentIterator(InsetBase * inset) const
{
	// this function re-creates the cache of inset pointers
	//lyxerr << "converting:\n" << *this << endl;
	DocumentIterator dit = DocumentIterator(*inset);
	for (size_t i = 0, n = data_.size(); i != n; ++i) {
		dit.push_back(data_[i]);
		dit.back().inset_ = inset;
		if (i + 1 != n)
			inset = dit.nextInset();
	}
	//lyxerr << "convert:\n" << *this << " to:\n" << dit << endl;
	return dit;
}


std::ostream & operator<<(std::ostream & os, StableDocumentIterator const & dit)
{
	for (size_t i = 0, n = dit.data_.size(); i != n; ++i)
		os << " " << dit.data_[i] << "\n";
	return os;
}

