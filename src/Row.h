// -*- C++ -*-
/**
 * \file Row.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Matthias Ettrich
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 *
 * Metrics for an on-screen text row.
 */

#ifndef ROW_H
#define ROW_H

#include "Changes.h"
#include "Dimension.h"
#include "Font.h"

#include "support/docstring.h"
#include "support/types.h"

#include <vector>

namespace lyx {

class DocIterator;
class Inset;

/**
 * An on-screen row of text. A paragraph is broken into a RowList for
 * display. Each Row contains a tokenized description of the contents
 * of the line.
 */
class Row {
public:
	// Possible types of row elements
	enum Type {
		// a string of character
		STRING,
		/**
		 * Something (completion, end-of-par marker)
		 * that occupies space one screen but does not
		 * correspond to any paragraph contents
		 */
		VIRTUAL,
		// An inset
		INSET,
		// Some spacing described by its width, not a string
		SPACE
	};

/**
 * One element of a Row. It has a set of attributes that can be used
 * by other methods that need to parse the Row contents.
 */
	struct Element {
		Element(Type const t, pos_type p, Font const & f, Change const & ch)
			: type(t), pos(p), endpos(p + 1), inset(0),
			  extra(0), font(f), change(ch), final(false) {}

		// Return the number of separator in the element (only STRING type)
		int countSeparators() const;

		// Return total width of element, including separator overhead
		// FIXME: Cache this value or the number of expanders?
		double full_width() const { return dim.wid + extra * countExpanders(); }
		// Return the number of expanding characters in the element (only STRING
		// type).
		int countExpanders() const;
		// Return the amount of expansion: the number of expanding characters
		// that get stretched during justification, times the em of the font
		// (only STRING type).
		int expansionAmount() const;
		// set extra proportionally to the font em value.
		void setExtra(double extra_per_em);

		/** Return position in pixels (from the left) of position
		 * \param i in the row element.
		 */
		double pos2x(pos_type const i) const;
		/** Return character position that is the closest to
		 *  pixel position \param x. The value \param x is
		 *  adjusted to the actual pixel position.
		*/
		pos_type x2pos(int &x) const;
		/** Break the element if possible, so that its width is less
		 * than \param w. Returns true on success. When \param force
		 * is true, the string is cut at any place, other wise it
		 * respects the row breaking rules of characters.
		 */
		bool breakAt(int w, bool force);

		// Returns the position on left side of the element.
		pos_type left_pos() const;
		// Returns the position on right side of the element.
		pos_type right_pos() const;

		//
		bool isRTL() const { return font.isVisibleRightToLeft(); }
		// This is true for virtual elements.
		// Note that we do not use the type here. The two definitions
		// should be equivalent
		bool isVirtual() const { return pos == endpos; }

		// The kind of row element
		Type type;
		// position of the element in the paragraph
		pos_type pos;
		// first position after the element in the paragraph
		pos_type endpos;
		// The dimension of the chunk (does not contains the
		// separator correction)
		Dimension dim;

		// Non-zero only if element is an inset
		Inset const * inset;

		// Only non-null for justified rows
		double extra;

		// Non-empty if element is a string or is virtual
		docstring str;
		//
		Font font;
		//
		Change change;
		// is it possible to add contents to this element?
		bool final;

		friend std::ostream & operator<<(std::ostream & os, Element const & row);
	};


	///
	Row();
	///
	bool changed() const { return changed_; }
	///
	void changed(bool c) const { changed_ = c; }
	/// Set the selection begin and end.
	/**
	  * This is const because we update the selection status only at draw()
	  * time.
	  */
	void setSelection(pos_type sel_beg, pos_type sel_end) const;
	///
	bool selection() const;
	/// Set the selection begin and end and whether the left and/or right
	/// margins are selected.
	void setSelectionAndMargins(DocIterator const & beg,
		DocIterator const & end) const;

	///
	void pit(pit_type p) { pit_ = p; }
	///
	pit_type pit() const { return pit_; }
	///
	void pos(pos_type p) { pos_ = p; }
	///
	pos_type pos() const { return pos_; }
	///
	void endpos(pos_type p) { end_ = p; }
	///
	pos_type endpos() const { return end_; }
	///
	void right_boundary(bool b) { right_boundary_ = b; }
	///
	bool right_boundary() const { return right_boundary_; }
	///
	void flushed(bool b) { flushed_ = b; }
	///
	bool flushed() const { return flushed_; }

	///
	Dimension const & dimension() const { return dim_; }
	///
	Dimension & dimension() { return dim_; }
	///
	int height() const { return dim_.height(); }
	/// The width of the row, including the left margin, but not the right one.
	int width() const { return dim_.wid; }
	///
	int ascent() const { return dim_.asc; }
	///
	int descent() const { return dim_.des; }

	/// The offset of the left-most cursor position on the row
	int left_x() const;
	/// The offset of the right-most cursor position on the row
	int right_x() const;

	// Return the number of separators in the row
	int countSeparators() const;
	// Set the extra spacing for every expanding character in STRING-type
	// elements.  \param w is the total amount of extra width for the row to be
	// distributed among expanders.  \return false if the justification fails.
	bool setExtraWidth(int w);

	///
	void add(pos_type pos, Inset const * ins, Dimension const & dim,
		 Font const & f, Change const & ch);
	///
	void add(pos_type pos, char_type const c,
		 Font const & f, Change const & ch);
	///
	void addVirtual(pos_type pos, docstring const & s,
			Font const & f, Change const & ch);
	///
	void addSpace(pos_type pos, int width, Font const & f, Change const & ch);

	///
	typedef std::vector<Element> Elements;
	///
	typedef Elements::iterator iterator;
	///
	typedef Elements::const_iterator const_iterator;
	///
	iterator begin() { return elements_.begin(); }
	///
	iterator end() { return elements_.end(); }
	///
	const_iterator begin() const { return elements_.begin(); }
	///
	const_iterator end() const { return elements_.end(); }

	///
	bool empty() const { return elements_.empty(); }
	///
	Element & front() { return elements_.front(); }
	///
	Element const & front() const { return elements_.front(); }
	///
	Element & back() { return elements_.back(); }
	///
	Element const & back() const { return elements_.back(); }
	/// remove last element
	void pop_back();
	/// remove all row elements
	void clear() { elements_.clear(); }
	/**
	 * if row width is too large, remove all elements after last
	 * separator and update endpos if necessary. If all that
	 * remains is a large word, cut it to \param width.
	 * \param body_pos minimum amount of text to keep.
	 * \param width maximum width of the row.
	 * \param available width on next row.
	 * \return true if the row has been shortened.
	 */
	bool shortenIfNeeded(pos_type const body_pos, int const width, int const next_width);

	/**
	 * If last element of the row is a string, compute its width
	 * and mark it final.
	 */
	void finalizeLast();

	/**
	 * Find sequences of right-to-left elements and reverse them.
	 * This should be called once the row is completely built.
	 */
	void reverseRTL(bool rtl_par);
	///
	bool isRTL() const { return rtl_; }
	///
	bool needsChangeBar() const { return changebar_; }
	///
	void needsChangeBar(bool ncb) { changebar_ = ncb; }

	/// Find row element that contains \c pos, and compute x offset.
	const_iterator const findElement(pos_type pos, bool boundary, double & x) const;

	friend std::ostream & operator<<(std::ostream & os, Row const & row);

	/// additional width for separators in justified rows (i.e. space)
	double separator;
	/// width of hfills in the label
	double label_hfill;
	/// the left margin position of the row
	int left_margin;
	/// the right margin of the row
	int right_margin;
	///
	mutable pos_type sel_beg;
	///
	mutable pos_type sel_end;
	///
	mutable bool begin_margin_sel;
	///
	mutable bool end_margin_sel;

private:
	/// Decides whether the margin is selected.
	/**
	  * \param margin_begin
	  * \param beg
	  * \param end
	  */
	bool isMarginSelected(bool left_margin, DocIterator const & beg,
		DocIterator const & end) const;

	/**
	 * Returns true if a char or string with font \c f and change
	 * type \c ch can be added to the current last element of the
	 * row.
	 */
	bool sameString(Font const & f, Change const & ch) const;

	///
	Elements elements_;

	/// has the Row appearance changed since last drawing?
	mutable bool changed_;
	/// Index of the paragraph that contains this row
	pit_type pit_;
	/// first pos covered by this row
	pos_type pos_;
	/// one behind last pos covered by this row
	pos_type end_;
	// Is there a boundary at the end of the row (display inset...)
	bool right_boundary_;
	// Shall the row be flushed when it is supposed to be justified?
	bool flushed_;
	/// Row dimension.
	Dimension dim_;
	/// true when this row lives in a right-to-left paragraph
	bool rtl_;
	/// true when a changebar should be drawn in the margin
	bool changebar_;
};


} // namespace lyx

#endif
