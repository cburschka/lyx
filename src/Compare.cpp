/**
 * \file Compare.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Vincent van Ravesteijn
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Compare.h"

#include "BufferParams.h"
#include "Changes.h"

#include "insets/InsetText.h"

#include "support/lassert.h"	

#include <boost/next_prior.hpp>

using namespace std;
using namespace lyx::support;


namespace lyx {

void step_forward(DocIterator & dit)
{
	dit.top().forwardPos();
}


void step_backward(DocIterator & dit)
{
	dit.top().backwardPos();
}


bool step_forward(DocIterator & dit, DocIterator const & end)
{
	if (dit == end)
		return false;
	step_forward(dit);
	return true;
}


bool step_backward(DocIterator & dit, DocIterator const & beg)
{
	if (dit == beg)
		return false;
	step_backward(dit);
	return true;
}

/**
 * A pair of two DocIterators that form a range.
 */
class DocRange {
public:
	DocRange(DocIterator from_, DocIterator to_)
		: from(from_), to(to_)
	{}

	DocRange(Buffer const * buf)
	{
		from = doc_iterator_begin(buf);
		to = doc_iterator_end(buf);
		to.backwardPos();
	}

	///
	Text * text() const { return from.text(); }
	///
	bool empty() const { return to <= from;	}
	///
	size_t length() const;

	/// The begin of the range
	DocIterator from;
	/// The end of the range
	DocIterator to;
};


size_t DocRange::length() const
{
	pit_type startpit = from.pit();
	pit_type endpit = to.pit();
	ParagraphList const & ps_ = from.text()->paragraphs();

	ParagraphList pars(boost::next(ps_.begin(), startpit),
				boost::next(ps_.begin(), endpit + 1));

	// Remove the end of the last paragraph; afterwards, remove the
	// beginning of the first paragraph.
	Paragraph & back = pars.back();
	back.eraseChars(to.pos(), back.size(), false);
	Paragraph & front = pars.front();
	front.eraseChars(0, from.pos(), false);

	ParagraphList::const_iterator pit = pars.begin();
	ParagraphList::const_iterator end_it = pars.end();

	size_t length = 0;
	for (; pit != end_it; ++pit)
		length += pit->size() + 1;

	// The last paragraph has no paragraph-end
	--length;
	return length;	
}


class DocPair {
public:
	DocPair() {}

	DocPair(DocIterator o_, DocIterator n_)
		: o(o_), n(n_)
	{}

	DocPair & operator++()
	{
		step_forward(o);
		step_forward(n);
		return *this;
	}
	///
	DocIterator o;
	///
	DocIterator n;
};
	
/**
 * A pair of two DocRanges.
 */
class DocRangePair {
public:
	DocRangePair(DocRange o_, DocRange n_)
		: o(o_), n(n_)
	{}
	
	DocRangePair(DocPair from, DocPair to)
		: o(from.o, to.o), n(from.n, to.n)
	{}

	DocRangePair(Buffer const * o_buf, Buffer const * n_buf)
		: o(o_buf), n(n_buf)
	{}

	/// Returns the from pair
	DocPair from() const { return DocPair(o.from, n.from); }

	/// Returns the to pair
	DocPair to() const { return DocPair(o.to, n.to); }

	DocRange o;
	DocRange n;
};


DocRangePair stepIntoInset(DocPair const & inset_location)
{
	DocRangePair rp(inset_location, inset_location);
	rp.o.from.forwardPos();
	rp.n.from.forwardPos();
	step_forward(rp.o.to);
	step_forward(rp.n.to);
	rp.o.to.backwardPos();
	rp.n.to.backwardPos();
	return rp;
}


/**
 * The implementation of the algorithm that does the comparison
 * between two documents.
 */
class Compare::Impl {
public:
	///
	Impl(Compare const & compare) 
		: abort_(false), compare_(compare)
	{}

	///
	~Impl() {}

	// Algorithm to find the shortest edit string. This algorithm 
	// only needs a linear amount of memory (linear with the sum
	// of the number of characters in the two paragraph-lists).
	bool diff(Buffer const * new_buf, Buffer const * old_buf,
		Buffer const * dest_buf);

	/// Set to true to cancel the algorithm
	bool abort_;

private:
	/// Finds the middle snake and returns the length of the
	/// shortest edit script.
	int find_middle_snake(DocRangePair const & rp, DocPair & middle_snake);

	/// This function is called recursively by a divide and conquer
	/// algorithm. Each time, the string is divided into two split
	/// around the middle snake.
	void diff_i(DocRangePair const & rp);

	/// Processes the splitted chunks. It either adds them as deleted,
	/// as added, or call diff_i for further processing.
	void diff_part(DocRangePair const & rp);

	/// Runs the algorithm for the inset located at /c it and /c it_n 
	/// and adds the result to /c pars.
	void diff_inset(Inset * inset, DocPair const & p);

	/// Adds the snake to the destination buffer. The algorithm will
	/// recursively be applied to any InsetTexts that are within the snake.
	void process_snake(DocRangePair const & rp);

	/// Writes the range to the destination buffer
	void writeToDestBuffer(DocRange const & range,
		Change::Type type = Change::UNCHANGED);
	
	/// Writes the paragraph list to the destination buffer
	void writeToDestBuffer(ParagraphList const & copy_pars) const;

	/// The length of the old chunk currently processed
	int N;
	/// The length of the new chunk currently processed
	int M;

	/// The thread object, used to emit signals to the GUI
	Compare const & compare_;

	/// The buffer containing text that will be marked as old
	Buffer const * old_buf_;
	/// The buffer containing text that will be marked as new
	Buffer const * new_buf_;
	/// The buffer containing text that will be marked as new
	Buffer const * dest_buf_;

	/// The paragraph list of the destination buffer
	ParagraphList * dest_pars_;

	/// The level of recursion
	int recursion_level_;

	/// The number of nested insets at this level
	int nested_inset_level_;
};

/////////////////////////////////////////////////////////////////////
//
// Compare
//
/////////////////////////////////////////////////////////////////////

Compare::Compare(Buffer const * new_buf, Buffer const * old_buf,
	Buffer * const dest_buf, CompareOptions const & options)
	: new_buffer(new_buf), old_buffer(old_buf), dest_buffer(dest_buf),
	  options_(options), pimpl_(new Impl(*this))
{
}


void Compare::run()
{
	if (!dest_buffer || !new_buffer || !old_buffer)
		return;

	// Copy the buffer params to the new buffer
	dest_buffer->params() = options_.settings_from_new
		? new_buffer->params() : old_buffer->params();
	
	// do the real work
	if (!doCompare())
		return;
	
	finished(pimpl_->abort_);
	return;
}


int Compare::doCompare()
{
	return pimpl_->diff(new_buffer, old_buffer, dest_buffer);
}


void Compare::abort()
{
	pimpl_->abort_ = true;
	condition_.wakeOne();
	wait();
	pimpl_->abort_ = false;
}


void get_paragraph_list(DocRange const & range,
	ParagraphList & pars)
{
	// Clone the paragraphs within the selection.
	pit_type startpit = range.from.pit();
	pit_type endpit = range.to.pit();
	ParagraphList const & ps_ = range.text()->paragraphs();
	ParagraphList tmp_pars(boost::next(ps_.begin(), startpit),
		boost::next(ps_.begin(), endpit + 1));

	// Remove the end of the last paragraph; afterwards, remove the
	// beginning of the first paragraph. Keep this order - there may only
	// be one paragraph!
	Paragraph & back = tmp_pars.back();
	back.eraseChars(range.to.pos(), back.size(), false);
	Paragraph & front = tmp_pars.front();
	front.eraseChars(0, range.from.pos(), false);

	pars.insert(pars.begin(), tmp_pars.begin(), tmp_pars.end());
}


bool equal(Inset const * i_o, Inset const * i_n)
{
	if (!i_o || !i_n)
		return false;

	// Different types of insets
	if (i_o->lyxCode() != i_n->lyxCode())
		return false;

	// Editable insets are assumed to be the same as they are of the 
	// same type. If we later on decide that we insert them in the
	// document as being unchanged, we will run the algorithm on the
	// contents of the two insets.
	// FIXME: This fails if the parameters of the insets differ.
	// FIXME: We do not recurse into InsetTabulars.
	// FIXME: We need methods inset->equivalent(inset).
	if (i_o->editable() && !i_o->asInsetMath()
		  && i_o->asInsetText())
		return true;

	ostringstream o_os;
	ostringstream n_os;
	i_o->write(o_os);
	i_n->write(n_os);
	return o_os.str() == n_os.str();
}


bool equal(DocIterator & o, DocIterator & n) {
	Paragraph const & old_par = o.text()->getPar(o.pit());
	Paragraph const & new_par = n.text()->getPar(n.pit());

	Inset const * i_o = old_par.getInset(o.pos());
	Inset const * i_n = new_par.getInset(n.pos());

	if (i_o && i_n)
		return equal(i_o, i_n);
	
	char_type c_o = old_par.getChar(o.pos());
	char_type c_n = new_par.getChar(n.pos());
	Font fo = old_par.getFontSettings(o.buffer()->params(), o.pos());
	Font fn = new_par.getFontSettings(n.buffer()->params(), n.pos());
	return c_o == c_n && fo == fn;
}


void traverse_snake_back(DocRangePair & rp)
{
	while (true) {
		// Traverse snake
		if (!step_backward(rp.o.to, rp.o.from))
			break;

		if (!step_backward(rp.n.to, rp.n.from)) {
			step_forward(rp.o.to);
			break;
		}

		if (!equal(rp.o.to, rp.n.to)) {
			step_forward(rp.o.to);
			step_forward(rp.n.to);
			break;
		}
	}
}


void traverse_snake_forw(DocRangePair & rp)
{
	while (equal(rp.o.from, rp.n.from)) {
		if (!step_forward(rp.o.from, rp.o.to))
			break;

		if (!step_forward(rp.n.from, rp.n.to)) {
			step_backward(rp.o.from);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////
//
// Compare::Impl
//
/////////////////////////////////////////////////////////////////////

int Compare::Impl::find_middle_snake(DocRangePair const & rp,
	DocPair &)
{
	N = rp.o.length();
	M = rp.n.length();
	return M+N;
}


bool Compare::Impl::diff(Buffer const * new_buf, Buffer const * old_buf,
	Buffer const * dest_buf)
{
	if (!new_buf || !old_buf || !dest_buf)
		return false;

	old_buf_ = old_buf;
	new_buf_ = new_buf;
	dest_buf_ = dest_buf;
	dest_pars_ = &dest_buf->inset().asInsetText()->paragraphs();
	dest_pars_->clear();

	recursion_level_ = 0;
	nested_inset_level_ = 0;

	DocRangePair rp(old_buf_, new_buf_);

	DocPair from = rp.from();
	traverse_snake_forw(rp);
	DocRangePair const snake(from, rp.from());
	process_snake(snake);
	
	// Start the recursive algorithm
	diff_i(rp);

	for (pit_type p = 0; p < (pit_type)dest_pars_->size(); ++p) {
		(*dest_pars_)[p].setBuffer(const_cast<Buffer &>(*dest_buf));
		(*dest_pars_)[p].setInsetOwner(&dest_buf_->inset());
	}

	return true;
}


void Compare::Impl::diff_i(DocRangePair const & rp)
{
	// The middle snake
	DocPair middle_snake;

	// Divides the problem into two smaller problems, split around
	// the snake in the middle.
	int const L_ses = find_middle_snake(rp, middle_snake);

	// Set maximum of progress bar
	if (++recursion_level_ == 1)
		compare_.progressMax(L_ses);

	// There are now three possibilities: the strings were the same,
	// the strings were completely different, or we found a middle
	// snake and we can split the string into two parts to process.
	if (L_ses == 0)
		// Two the same strings (this must be a very rare case, because
		// usually this will be part of a snake adjacent to these strings).
		writeToDestBuffer(rp.o);

	else if (middle_snake.o.empty()) {
		// Two totally different strings
		writeToDestBuffer(rp.o, Change::DELETED);
		writeToDestBuffer(rp.n, Change::INSERTED);

	} else {
		// Retrieve the complete snake
		DocRangePair first_part(rp.from(), middle_snake);
		traverse_snake_back(first_part);
			
		DocRangePair second_part(middle_snake, rp.to());
		traverse_snake_forw(second_part);
			
		// Split the string in three parts:
		// 1. in front of the snake
		diff_part(first_part);

		// 2. the snake itself, and
		DocRangePair const snake(first_part.to(), second_part.from());
		process_snake(snake);

		// 3. behind the snake.
		diff_part(second_part);
	}
	--recursion_level_;
}


void Compare::Impl::diff_part(DocRangePair const & rp)
{
	// Is there a finite length string in both buffers, if not there
	// is an empty string and we write the other one to the buffer.
	if (!rp.o.empty() && !rp.n.empty())
		diff_i(rp);
	
	else if (!rp.o.empty())
		writeToDestBuffer(rp.o, Change::DELETED);

	else if (!rp.n.empty())
		writeToDestBuffer(rp.n, Change::INSERTED);
}


void Compare::Impl::diff_inset(Inset * inset, DocPair const & p)
{
	// Find the dociterators for the beginning and the
	// end of the inset, for the old and new document.
	DocRangePair const rp = stepIntoInset(p);

	// Recurse into the inset. Temporarily replace the dest_pars
	// paragraph list by the paragraph list of the nested inset.
	ParagraphList * backup_dest_pars = dest_pars_;
	dest_pars_ = &inset->asInsetText()->text().paragraphs();
	dest_pars_->clear();
	
	++nested_inset_level_;
	diff_i(rp);
	--nested_inset_level_;

	dest_pars_ = backup_dest_pars;
}


void Compare::Impl::process_snake(DocRangePair const & rp)
{
	ParagraphList pars;
	get_paragraph_list(rp.o, pars);

	// Find insets in this paragaph list
	DocPair it = rp.from();
	for (; it.o < rp.o.to; ++it) {
		Inset * inset = it.o.text()->getPar(it.o.pit()).getInset(it.o.pos());
		if (inset && inset->editable() && inset->asInsetText()) {
			// Find the inset in the paragraph list that will be pasted into
			// the final document. The contents of the inset will be replaced
			// by the output of the algorithm below.
			pit_type const pit = it.o.pit() - rp.o.from.pit();
			pos_type const pos = pit ? it.o.pos() : it.o.pos() - rp.o.from.pos();
			inset = pars[pit].getInset(pos);
			LASSERT(inset, /**/);
			diff_inset(inset, it);
		}
	}
	writeToDestBuffer(pars);
}


void Compare::Impl::writeToDestBuffer(DocRange const & range,
	Change::Type type)
{
	ParagraphList pars;
	get_paragraph_list(range, pars);

	pos_type size = 0;

	// Set the change
	ParagraphList::iterator it = pars.begin();
	for (; it != pars.end(); ++it) {
		it->setChange(Change(type));
		size += it->size();
	}

	writeToDestBuffer(pars);

	if (nested_inset_level_ == 0)
		compare_.progress(size);
}


void Compare::Impl::writeToDestBuffer(ParagraphList const & pars) const
{
	pit_type const pit = dest_pars_->size() - 1;
	dest_pars_->insert(dest_pars_->end(), pars.begin(), pars.end());
	if (pit >= 0)
		mergeParagraph(dest_buf_->params(), *dest_pars_, pit);
}


#include "moc_Compare.cpp"

} // namespace lyx
