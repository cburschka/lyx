// -*- C++ -*-
/**
 * \file DocIterator.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DOCITERATOR_H
#define DOCITERATOR_H

#include "CursorSlice.h"

#include <vector>
#include <algorithm>

namespace lyx {

class DocIterator;
class LyXErr;
class MathAtom;
class Paragraph;
class Text;
class InsetIterator;
class FontSpan;

DocIterator doc_iterator_begin(Buffer const * buf, Inset const * inset = 0);
DocIterator doc_iterator_end(Buffer const * buf, Inset const * inset = 0);


class DocIterator
{
public:
	/// type for cell number in inset
	typedef CursorSlice::idx_type idx_type;
	/// type for row indices
	typedef CursorSlice::row_type row_type;
	/// type for col indices
	typedef CursorSlice::col_type col_type;

public:
	///
	DocIterator();
	///
	explicit DocIterator(Buffer *buf);

	/// access to owning buffer
	Buffer * buffer() const { return buffer_; }
	/// access to owning buffer
	void setBuffer(Buffer * buf) { buffer_ = buf; }

	/// Clone this for given \p buffer.
	/// \p buffer must be a clone of buffer_.
	DocIterator clone(Buffer * buffer) const;

	/// access slice at position \p i
	CursorSlice const & operator[](size_t i) const { return slices_[i]; }
	/// access slice at position \p i
	CursorSlice & operator[](size_t i) { return slices_[i]; }
	/// chop a few slices from the iterator
	void resize(size_t i) { slices_.resize(i); }

	/// is the iterator valid?
	operator const void*() const { return empty() ? 0 : this; }
	/// is this iterator invalid?
	bool operator!() const { return empty(); }

	/// does this iterator have any content?
	bool empty() const { return slices_.empty(); }
	/// is this the end position?
	bool atEnd() const { return slices_.empty(); }
	/// is this the last possible position?
	bool atLastPos() const { return pit() == lastpit() && pos() == lastpos(); }

	/// checks the cursor slices for disabled spell checker insets
	bool allowSpellCheck() const;

	//
	// access to slice at tip
	//
	/// access to tip
	CursorSlice & top() { return slices_.back(); }
	/// access to tip
	CursorSlice const & top() const { return slices_.back(); }
	/// access to outermost slice
	CursorSlice & bottom() { return slices_.front(); }
	/// access to outermost slice
	CursorSlice const & bottom() const { return slices_.front(); }
	/// how many nested insets do we have?
	size_t depth() const { return slices_.size(); }
	/// the containing inset
	Inset & inset() const { return top().inset(); }
	/// return the cell of the inset this cursor is in
	idx_type idx() const { return top().idx(); }
	/// return the cell of the inset this cursor is in
	idx_type & idx() { return top().idx(); }
	/// return the last possible cell in this inset
	idx_type lastidx() const;
	/// return the paragraph this cursor is in
	pit_type pit() const { return top().pit(); }
	/// return the paragraph this cursor is in
	pit_type & pit() { return top().pit(); }
	/// return the last possible paragraph in this inset
	pit_type lastpit() const;
	/// return the position within the paragraph
	pos_type pos() const { return top().pos(); }
	/// return the position within the paragraph
	pos_type & pos() { return top().pos(); }
	/// return the last position within the paragraph
	pos_type lastpos() const;

	/// return the number of embedded cells
	size_t nargs() const;
	/// return the number of embedded cells
	size_t ncols() const;
	/// return the number of embedded cells
	size_t nrows() const;
	/// return the grid row of the top cell
	row_type row() const;
	/// return the last row of the top grid
	row_type lastrow() const { return nrows() - 1; }
	/// return the grid column of the top cell
	col_type col() const;
	/// return the last column of the top grid
	col_type lastcol() const { return ncols() - 1; }
	/// the inset just behind the cursor
	Inset * nextInset() const;
	/// the inset just in front of the cursor
	Inset * prevInset() const;
	///
	bool boundary() const { return boundary_; }
	///
	void boundary(bool b) { boundary_ = b; }

	// the two methods below have been inlined out because of
	// profiling results under linux when opening a document.
	/// are we in mathed?.
	bool inMathed() const
	{ return !empty() && inset().inMathed(); }
	/// are we in texted?.
	bool inTexted() const
	{ return !empty() && !inset().inMathed(); }
	/// are we in regexp-mode ?
	bool inRegexped() const;

	//
	// math-specific part
	//
	/// return the mathed cell this cursor is in
	MathData & cell() const;
	/// the mathatom left of the cursor
	MathAtom & prevAtom() const;
	/// the mathatom right of the cursor
	MathAtom & nextAtom() const;

	// text-specific part
	//
	/// the paragraph we're in in text mode.
	/// \warning only works within text!
	Paragraph & paragraph() const;
	/// the paragraph we're in in any case.
	/// This method will give the containing paragraph even
	/// if not in text mode (ex: in mathed).
	Paragraph & innerParagraph() const;
	/// return the inner text slice.
	CursorSlice const & innerTextSlice() const;
	/// returns a DocIterator for the containing text inset
	DocIterator getInnerText() const;
	/// the first and last positions of a word at top cursor slice
	/// \warning only works within text!
	FontSpan locateWord(word_location const loc) const;
	///
	Text * text() const;
	/// the containing inset or the cell, respectively
	Inset * realInset() const;
	///
	Inset * innerInsetOfType(int code) const;
	///
	Text * innerText() const;

	//
	// elementary moving
	//
	/**
	 * move on one logical position, descend into nested insets
	 * including collapsed insets
	 */
	void forwardPos();
	/**
	 * move on one logical position, descend into nested insets
	 * skip collapsed insets
	 */
	void forwardPosIgnoreCollapsed();
	/// move on one physical character or inset
	void forwardChar();
	/// move on one paragraph
	void forwardPar();
	/// move on one inset
	void forwardInset();
	/// move backward one logical position
	void backwardPos();
	/// move backward one physical character or inset
	void backwardChar();
	/// move backward one paragraph
	void backwardPar();
	/// move backward one inset
	/// not used currently, uncomment if you need it
	//void backwardInset();

	/// are we some 'extension' (i.e. deeper nested) of the given iterator
	bool hasPart(DocIterator const & it) const;

	/// output
	friend std::ostream &
	operator<<(std::ostream & os, DocIterator const & cur);
	friend LyXErr & operator<<(LyXErr & os, DocIterator const & it);
	///
	friend bool operator==(DocIterator const &, DocIterator const &);
	friend bool operator<(DocIterator const &, DocIterator const &);
	friend bool operator>(DocIterator const &, DocIterator const &);
	friend bool operator<=(DocIterator const &, DocIterator const &);
	///
	friend class StableDocIterator;
//protected:
	///
	void clear() { slices_.clear(); }
	///
	void push_back(CursorSlice const & sl) { slices_.push_back(sl); }
	///
	void pop_back() { slices_.pop_back(); }
	/// recompute the inset parts of the cursor from the document data
	void updateInsets(Inset * inset);
	/// fix DocIterator in circumstances that should never happen.
	/// \return true if the DocIterator was fixed.
	bool fixIfBroken();
	/// Repopulate the slices insets from bottom to top. Useful
	/// for stable iterators or Undo data.
	void sanitize();

	/// find index of CursorSlice with &cell() == &cell (or -1 if not found)
	int find(MathData const & cell) const;
	/// find index of CursorSlice with inset() == inset (or -1 of not found)
	int find(Inset const * inset) const;
	/// cut off CursorSlices with index > above and store cut off slices in cut.
	void cutOff(int above, std::vector<CursorSlice> & cut);
	/// cut off CursorSlices with index > above
	void cutOff(int above);
	/// push CursorSlices on top
	void append(std::vector<CursorSlice> const & x);
	/// push one CursorSlice on top and set its index and position
	void append(idx_type idx, pos_type pos);

private:
	friend class InsetIterator;
	friend DocIterator doc_iterator_begin(Buffer const * buf, Inset const * inset);
	friend DocIterator doc_iterator_end(Buffer const * buf, Inset const * inset);
	///
	explicit DocIterator(Buffer * buf, Inset * inset);
	/**
	 * Normally, when the cursor is at position i, it is painted *before*
	 * the character at position i. However, what if we want the cursor 
	 * painted *after* position i? That's what boundary_ is for: if
	 * boundary_==true, the cursor is painted *after* position i-1, instead
	 * of before position i.
	 *
	 * Note 1: Usually, after i-1 or before i are actually the same place!
	 * However, this is not the case when i-1 and i are not painted 
	 * contiguously, and in these cases we sometimes do want to have control
	 * over whether to paint before i or after i-1.
	 * Some concrete examples of where this happens:
	 * a. i-1 at the end of one row, i at the beginning of next row
	 * b. in bidi text, at transitions between RTL and LTR or vice versa
	 *
	 * Note 2: Why i and i-1? Why, if boundary_==false means: *before* i, 
	 * couldn't boundary_==true mean: *after* i? 
	 * Well, the reason is this: cursor position is not used only for 
	 * painting the cursor, but it also affects other things, for example:
	 * where the next insertion will be placed (it is inserted at the current
	 * position, pushing anything at the current position and beyond forward).
	 * Now, when the current position is i and boundary_==true, insertion would
	 * happen *before* i. If the cursor, however, were painted *after* i, that
	 * would be very unnatural...
	 */
	bool boundary_;
	///
	std::vector<CursorSlice> const & internalData() const { return slices_; }
	///
	std::vector<CursorSlice> slices_;
	///
	Inset * inset_;
	///
	Buffer * buffer_;
};


inline bool operator==(DocIterator const & di1, DocIterator const & di2)
{
	return di1.slices_ == di2.slices_;
}


inline bool operator!=(DocIterator const & di1, DocIterator const & di2)
{
	return !(di1 == di2);
}


inline
bool operator<(DocIterator const & p, DocIterator const & q)
{
	size_t depth = std::min(p.depth(), q.depth());
	for (size_t i = 0 ; i < depth ; ++i) {
		if (p[i] != q[i])
			return p[i] < q[i];
	}
	return p.depth() < q.depth();
}


inline	
bool operator>(DocIterator const & p, DocIterator const & q)
{
	return q < p;
}


inline	
bool operator<=(DocIterator const & p, DocIterator const & q)
{
	return !(q < p);
}


inline	
bool operator>=(DocIterator const & p, DocIterator const & q)
{
	return !(p < q);
}


// The difference to a ('non stable') DocIterator is the removed
// (overwritten by 0...) part of the CursorSlice data items. So this thing
// is suitable for external storage, but not for iteration as such.

class StableDocIterator
{
public:
	///
	StableDocIterator() {}
	/// non-explicit intended
	StableDocIterator(const DocIterator & it);
	///
	DocIterator asDocIterator(Buffer * buf) const;
	///
	size_t size() const { return data_.size(); }
	///  return the position within the paragraph
	pos_type pos() const { return data_.back().pos(); }
	///  return the position within the paragraph
	pos_type & pos() { return data_.back().pos(); }
	///
	friend std::ostream &
	operator<<(std::ostream & os, StableDocIterator const & cur);
	///
	friend std::istream &
	operator>>(std::istream & is, StableDocIterator & cur);
	///
	friend bool
	operator==(StableDocIterator const &, StableDocIterator const &);
private:
	std::vector<CursorSlice> data_;
};

} // namespace lyx

#endif // DOCITERATOR_H
