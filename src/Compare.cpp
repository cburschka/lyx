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

#include "Author.h"
#include "BufferParams.h"
#include "Changes.h"
#include "Font.h"

#include "insets/InsetText.h"

#include "support/lassert.h"
#include "support/lyxalgo.h"
#include "support/qstring_helpers.h"

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
	DocRange(DocIterator const & from_, DocIterator const & to_)
		: from(from_), to(to_)
	{}

	DocRange(Buffer const * buf) :
		from(doc_iterator_begin(buf)),
		to(doc_iterator_end(buf))
	{
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
	ParagraphList const & ps = from.text()->paragraphs();
	size_t length = 0;
	pit_type pit = from.pit();
	pit_type const endpit = to.pit();
	for (; pit < endpit; ++pit)
		length += ps[pit].size() + 1;
	length += to.pos() - from.pos();
	return length;
}


class DocPair {
public:
	DocPair()
	{}

	DocPair(DocIterator o_, DocIterator n_)
		: o(o_), n(n_)
	{}

	bool operator!=(DocPair const & rhs)
	{
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
	DocRangePair(DocRange const & o_, DocRange const & n_)
		: o(o_), n(n_)
	{}

	DocRangePair(DocPair const & from, DocPair const & to)
		: o(from.o, to.o), n(from.n, to.n)
	{}

	DocRangePair(Buffer const * o_buf, Buffer const * n_buf)
		: o(o_buf), n(n_buf)
	{}

	/// Returns the from pair
	DocPair from() const
	{
		return DocPair(o.from, n.from);
	}

	/// Returns the to pair
	DocPair to() const
	{
		return DocPair(o.to, n.to);
	}

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
	compl_vector()
	{}

	void reset(T const & def)
	{
		default_ = def;
		Vp_.clear();
		Vn_.clear();
	}

	/// Gets the value at index. If it is not in the vector
	/// the default value is inserted and returned.
	T & operator[](int index) {
		vector<T> & V = index >= 0 ? Vp_ : Vn_;
		unsigned int const ii = index >= 0 ? index : -index - 1;
		while (ii >= V.size())
			V.push_back(default_);
		return V[ii];
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
		: abort_(false), n_(0), m_(0), offset_reverse_diagonal_(0),
		  odd_offset_(0), compare_(compare),
		  old_buf_(0), new_buf_(0), dest_buf_(0), dest_pars_(0),
		  recursion_level_(0), nested_inset_level_(0), D_(0)
	{}

	///
	~Impl()
	{}

	// Algorithm to find the shortest edit string. This algorithm
	// only needs a linear amount of memory (linear with the sum
	// of the number of characters in the two paragraph-lists).
	bool diff(Buffer const * new_buf, Buffer const * old_buf,
		Buffer const * dest_buf);

	/// Set to true to cancel the algorithm
	bool abort_;

	///
	QString status()
	{
		QString status;
		status += toqstr("recursion level:") + " " + QString::number(recursion_level_)
			+ " " + toqstr("differences:") + " " + QString::number(D_);
		return status;
	}

private:
	/// Finds the middle snake and returns the length of the
	/// shortest edit script.
	int findMiddleSnake(DocRangePair const & rp, DocPair & middle_snake);

	enum SnakeResult {
		NoSnake,
		SingleSnake,
		NormalSnake
	};

	/// Retrieve the middle snake when there is overlap between
	/// the forward and backward path.
	SnakeResult retrieveMiddleSnake(int k, int D, Direction direction,
		DocPair & middle_snake);

	/// Find the furthest reaching D-path (number of horizontal
	/// and vertical steps; differences between the old and new
	/// document) in the k-diagonal (vertical minus horizontal steps).
	void furthestDpathKdiagonal(int D, int k,
		DocRangePair const & rp, Direction direction);

	/// Is there overlap between the forward and backward path
	bool overlap(int k, int D);

	/// This function is called recursively by a divide and conquer
	/// algorithm. Each time, the string is divided into two split
	/// around the middle snake.
	void diff_i(DocRangePair const & rp);

	/// Processes the split chunks. It either adds them as deleted,
	/// as added, or call diff_i for further processing.
	void diffPart(DocRangePair const & rp);

	/// Runs the algorithm for the inset located at /c it and /c it_n
	/// and adds the result to /c pars.
	void diffInset(Inset * inset, DocPair const & p);

	/// Adds the snake to the destination buffer. The algorithm will
	/// recursively be applied to any InsetTexts that are within the snake.
	void processSnake(DocRangePair const & rp);

	/// Writes the range to the destination buffer
	void writeToDestBuffer(DocRange const & range,
		Change::Type type = Change::UNCHANGED);

	/// Writes the paragraph list to the destination buffer
	void writeToDestBuffer(ParagraphList const & copy_pars) const;

	/// The length of the old chunk currently processed
	int n_;
	/// The length of the new chunk currently processed
	int m_;
	/// The offset diagonal of the reverse path of the
	/// currently processed chunk
	int offset_reverse_diagonal_;
	/// Is the offset odd or even ?
	bool odd_offset_;

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

	/// The position/snake in the old/new document
	/// of the forward/reverse search
	compl_vector<DocIterator> ofp;
	compl_vector<DocIterator> nfp;
	compl_vector<DocIterator> ofs;
	compl_vector<DocIterator> nfs;
	compl_vector<DocIterator> orp;
	compl_vector<DocIterator> nrp;
	compl_vector<DocIterator> ors;
	compl_vector<DocIterator> nrs;

	/// The number of differences in the path the algorithm
	/// is currently processing.
	int D_;
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
	connect(&status_timer_, SIGNAL(timeout()),
		this, SLOT(doStatusMessage()));
	status_timer_.start(1000);
}


void Compare::doStatusMessage()
{
	statusMessage(pimpl_->status());
}


void Compare::run()
{
	if (!dest_buffer || !new_buffer || !old_buffer)
		return;

	// Copy the buffer params to the destination buffer
	dest_buffer->params() = options_.settings_from_new
		? new_buffer->params() : old_buffer->params();

	// Copy extra authors to the destination buffer
	AuthorList const & extra_authors = options_.settings_from_new ?
		old_buffer->params().authors() : new_buffer->params().authors();
	AuthorList::Authors::const_iterator it = extra_authors.begin();
	for (; it != extra_authors.end(); ++it)
		dest_buffer->params().authors().record(*it);

	doStatusMessage();

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


static void getParagraphList(DocRange const & range,
	ParagraphList & pars)
{
	// Clone the paragraphs within the selection.
	pit_type startpit = range.from.pit();
	pit_type endpit = range.to.pit();
	ParagraphList const & ps_ = range.text()->paragraphs();
	ParagraphList tmp_pars(lyx::next(ps_.begin(), startpit),
		lyx::next(ps_.begin(), endpit + 1));

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


static bool equal(DocIterator & o, DocIterator & n)
{
	// Explicitly check for this, so we won't call
	// Paragraph::getChar for the last pos.
	bool const o_lastpos = o.pos() == o.lastpos();
	bool const n_lastpos = n.pos() == n.lastpos();
	if (o_lastpos || n_lastpos)
		return o_lastpos && n_lastpos;

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
static bool traverseSnake(DocPair & p, DocRangePair const & range,
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


void Compare::Impl::furthestDpathKdiagonal(int D, int k,
	 DocRangePair const & rp, Direction direction)
{
	compl_vector<DocIterator> & op = direction == Forward ? ofp : orp;
	compl_vector<DocIterator> & np = direction == Forward ? nfp : nrp;
	compl_vector<DocIterator> & os = direction == Forward ? ofs : ors;
	compl_vector<DocIterator> & ns = direction == Forward ? nfs : nrs;

	// A vertical step means stepping one character in the new document.
	bool vertical_step = k == -D;
	if (!vertical_step && k != D) {
		vertical_step = direction == Forward
			? op[k - 1] < op[k + 1] : op[k - 1] > op[k + 1];
	}

	// Where do we take the step from ?
	int const kk = vertical_step ? k + 1 : k - 1;
	DocPair p(op[kk], np[kk]);
	DocPair const s(os[kk], ns[kk]);

	// If D==0 we simulate a vertical step from (0,-1) by doing nothing.
	if (D != 0) {
		// Take a step
		if (vertical_step && direction == Forward)
			step(p.n, rp.n.to, direction);
		else if (vertical_step && direction == Backward)
			step(p.n, rp.n.from, direction);
		else if (!vertical_step && direction == Forward)
			step(p.o, rp.o.to, direction);
		else if (!vertical_step && direction == Backward)
			step(p.o, rp.o.from, direction);
	}

	// Traverse snake
	if (traverseSnake(p, rp, direction)) {
		// Record last snake
		os[k] = p.o;
		ns[k] = p.n;
	} else {
		// Copy last snake from the previous step
		os[k] = s.o;
		ns[k] = s.n;
	}

	//Record new position
	op[k] = p.o;
	np[k] = p.n;
}


bool Compare::Impl::overlap(int k, int D)
{
	// To generalize for the forward and reverse checks
	int kk = offset_reverse_diagonal_ - k;

	// Can we have overlap ?
	if (kk <= D && kk >= -D) {
		// Do we have overlap ?
		if (odd_offset_)
			return ofp[k] >= orp[kk] && nfp[k] >= nrp[kk];
		else
			return ofp[kk] >= orp[k] && nfp[kk] >= nrp[k];
	}
	return false;
}


Compare::Impl::SnakeResult Compare::Impl::retrieveMiddleSnake(
	int k, int D, Direction direction, DocPair & middle_snake)
{
	compl_vector<DocIterator> & os = direction == Forward ? ofs : ors;
	compl_vector<DocIterator> & ns = direction == Forward ? nfs : nrs;
	compl_vector<DocIterator> & os_r = direction == Forward ? ors : ofs;
	compl_vector<DocIterator> & ns_r = direction == Forward ? nrs : nfs;

	// The diagonal while doing the backward search
	int kk = -k + offset_reverse_diagonal_;

	// Did we find a snake ?
	if (os[k].empty() && os_r[kk].empty()) {
		// No, there is no snake at all, in which case
		// the length of the shortest edit script is M+N.
		LATTEST(2 * D - odd_offset_ == m_ + n_);
		return NoSnake;
	}

	if (os[k].empty()) {
		// Yes, but there is only 1 snake and we found it in the
		// reverse path.
		middle_snake.o = os_r[kk];
		middle_snake.n = ns_r[kk];
		return SingleSnake;
	}

	middle_snake.o = os[k];
	middle_snake.n = ns[k];
	return NormalSnake;
}


int Compare::Impl::findMiddleSnake(DocRangePair const & rp,
	DocPair & middle_snake)
{
	// The lengths of the old and new chunks.
	n_ = rp.o.length();
	m_ = rp.n.length();

	// Forward paths are centered around the 0-diagonal; reverse paths
	// are centered around the diagonal N - M. (Delta in the article)
	offset_reverse_diagonal_ = n_ - m_;

	// If the offset is odd, only check for overlap while extending forward
    // paths, otherwise only check while extending reverse paths.
	odd_offset_ = (offset_reverse_diagonal_ % 2 != 0);

	ofp.reset(rp.o.from);
	nfp.reset(rp.n.from);
	ofs.reset(DocIterator());
	nfs.reset(DocIterator());
	orp.reset(rp.o.to);
	nrp.reset(rp.n.to);
	ors.reset(DocIterator());
	nrs.reset(DocIterator());

	// In the formula below, the "+ 1" ensures we round like ceil()
	int const D_max = (m_ + n_ + 1)/2;
	// D is the number of horizontal and vertical steps, i.e.
	// different characters in the old and new chunk.
	for (int D = 0; D <= D_max; ++D) {
		// to be used in the status messages
		D_ = D;

		// Forward and reverse paths
		for (int f = 0; f < 2; ++f) {
			Direction direction = f == 0 ? Forward : Backward;

			// Diagonals between -D and D can be reached by a D-path
			for (int k = -D; k <= D; k += 2) {
				// Find the furthest reaching D-path on this diagonal
				furthestDpathKdiagonal(D, k, rp, direction);

				// Only check for overlap for forward paths if the offset is odd
				// and only for reverse paths if the offset is even.
				if (odd_offset_ == (direction == Forward)) {

					// Do the forward and backward paths overlap ?
					if (overlap(k, D - odd_offset_)) {
						retrieveMiddleSnake(k, D, direction, middle_snake);
						return 2 * D - odd_offset_;
					}
				}
				if (abort_)
					return 0;
			}
		}
	}
	// This should never be reached
	return -2;
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
	traverseSnake(from, rp, Forward);
	DocRangePair const snake(rp.from(), from);
	processSnake(snake);

	// Start the recursive algorithm
	DocRangePair rp_new(from, rp.to());
	if (!rp_new.o.empty() || !rp_new.n.empty())
		diff_i(rp_new);

	for (pit_type p = 0; p < (pit_type)dest_pars_->size(); ++p) {
		(*dest_pars_)[p].setBuffer(const_cast<Buffer &>(*dest_buf));
		(*dest_pars_)[p].setInsetOwner(&dest_buf_->inset());
	}

	return true;
}


void Compare::Impl::diff_i(DocRangePair const & rp)
{
	if (abort_)
		return;

	// The middle snake
	DocPair middle_snake;

	// Divides the problem into two smaller problems, split around
	// the snake in the middle.
	int const L_ses = findMiddleSnake(rp, middle_snake);

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
		traverseSnake(first_part_end, rp, Backward);
		DocRangePair first_part(rp.from(), first_part_end);

		DocPair second_part_begin = middle_snake;
		traverseSnake(second_part_begin, rp, Forward);
		DocRangePair second_part(second_part_begin, rp.to());

		// Split the string in three parts:
		// 1. in front of the snake
		diffPart(first_part);

		// 2. the snake itself, and
		DocRangePair const snake(first_part.to(), second_part.from());
		processSnake(snake);

		// 3. behind the snake.
		diffPart(second_part);
	}
	--recursion_level_;
}


void Compare::Impl::diffPart(DocRangePair const & rp)
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


void Compare::Impl::diffInset(Inset * inset, DocPair const & p)
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


void Compare::Impl::processSnake(DocRangePair const & rp)
{
	ParagraphList pars;
	getParagraphList(rp.o, pars);

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
			LASSERT(inset, continue);
			diffInset(inset, it);
		}
	}
	writeToDestBuffer(pars);
}


void Compare::Impl::writeToDestBuffer(DocRange const & range,
	Change::Type type)
{
	ParagraphList pars;
	getParagraphList(range, pars);

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
