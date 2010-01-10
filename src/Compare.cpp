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


enum Direction {
	Forward = 0,
	Backward
};


static void step(DocIterator & dit, Direction direction)
{
	if (direction == Forward)
		dit.top().forwardPos();
	else
		dit.top().backwardPos();
}


static void step(DocIterator & dit, DocIterator const & end, Direction direction)
{
	if (dit != end)
		step(dit, direction);
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

	bool operator!=(DocPair const & rhs) {
		// this might not be intuitive but correct for our purpose
		return o != rhs.o && n != rhs.n;
	}
	

	DocPair & operator++()
	{
		step(o, Forward);
		step(n, Forward);
		return *this;
	}

	DocPair & operator--()
	{
		step(o, Backward);
		step(n, Backward);
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


static DocRangePair stepIntoInset(DocPair const & inset_location)
{
	DocRangePair rp(inset_location, inset_location);
	rp.o.from.forwardPos();
	rp.n.from.forwardPos();
	step(rp.o.to, Forward);
	step(rp.n.to, Forward);
	rp.o.to.backwardPos();
	rp.n.to.backwardPos();
	return rp;
}


/**
 *  This class is designed to hold a vector that has both positive as
 *  negative indices. It is internally represented as two vectors, one
 *  for non-zero indices and one for negative indices. In this way, the
 *  vector can grow in both directions.
 *    If an index is not available in the vector, the default value is
 *  returned. If an object is put in the vector beyond its size, the
 *  empty spots in between are also filled with the default value.
 */
template<class T>
class compl_vector {
public:
	compl_vector() {}

	void reset(T const & def)
	{
		default_ = def;
		Vp_.clear();
		Vn_.clear();
	}


	/// Gets the value at index. If it is not in the vector
	/// the default value is returned.
	T & get(int index) {
		if (-index <= int(Vn_.size()) && index < int(Vp_.size()))
			return index >= 0 ? Vp_[index] : Vn_[-index-1];
		else
			return default_;
	}

	/// Sets the value at index if it already
	/// is in the vector. Otherwise it will be added to the
	/// end padded with the default value.
	void set(int index, T const & t) {
		if (index >= -int(Vn_.size()) && index < int(Vp_.size())) {
			if (index >= 0)
				Vp_[index] = t;
			else 
				Vn_[-index-1] = t;
		} else {
			while (index > int(Vp_.size()))
				Vp_.push_back(default_);
			while (index < -int(Vn_.size()) - 1)
				Vn_.push_back(default_);

			if (index >= 0)
				Vp_.push_back(t);
			else
				Vn_.push_back(t);
		}
	}

private:
	/// The vector for positive indices
	vector<T> Vp_;
	/// The vector for negative indices
	vector<T> Vn_;
	/// The default value that is inserted in the vector
	/// if more space is needed
	T default_;
};


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
	int N_;
	/// The length of the new chunk currently processed
	int M_;

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


static void get_paragraph_list(DocRange const & range,
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


static bool equal(Inset const * i_o, Inset const * i_n)
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


static bool equal(DocIterator & o, DocIterator & n) {
	Paragraph const & old_par = o.text()->getPar(o.pit());
	Paragraph const & new_par = n.text()->getPar(n.pit());

	char_type const c_o = old_par.getChar(o.pos());
	char_type const c_n = new_par.getChar(n.pos());
	if (c_o != c_n)
		return false;

	if (old_par.isInset(o.pos())) {
		Inset const * i_o = old_par.getInset(o.pos());
		Inset const * i_n = new_par.getInset(n.pos());

		if (i_o && i_n)
			return equal(i_o, i_n);
	}	

	Font fo = old_par.getFontSettings(o.buffer()->params(), o.pos());
	Font fn = new_par.getFontSettings(n.buffer()->params(), n.pos());
	return fo == fn;
}


/// Traverses a snake in a certain direction. p points to a 
/// position in the old and new file and they are synchronously
/// moved along the snake. The function returns true if a snake
/// was found.
static bool traverse_snake(DocPair & p, DocRangePair const & range,
	Direction direction)
{
	bool ret = false;
	DocPair const & p_end = 
		direction == Forward ? range.to() : range.from();

	while (p != p_end) {
		if (direction == Backward)
			--p;
		if (!equal(p.o, p.n)) {
			if (direction == Backward)
				++p;
			return ret;
		}
		if (direction == Forward)
			++p;
		ret = true;
	}
	return ret;
}


/////////////////////////////////////////////////////////////////////
//
// Compare::Impl
//
/////////////////////////////////////////////////////////////////////

int Compare::Impl::find_middle_snake(DocRangePair const & rp,
	DocPair &)
{
	N_ = rp.o.length();
	M_ = rp.n.length();
	return M_ + N_;
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
	traverse_snake(from, rp, Forward);
	DocRangePair const snake(rp.from(), from);
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
		DocPair first_part_end = middle_snake;
		traverse_snake(first_part_end, rp, Backward);
		DocRangePair first_part(rp.from(), first_part_end);
			
		DocPair second_part_begin = middle_snake;
		traverse_snake(second_part_begin, rp, Forward);
		DocRangePair second_part(second_part_begin, rp.to());
				
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
