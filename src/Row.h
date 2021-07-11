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
		SPACE,
		// Something that should not happen (for error handling)
		INVALID
	};

/**
 * One element of a Row. It has a set of attributes that can be used
 * by other methods that need to parse the Row contents.
 */
	struct Element {
		//
		Element() = default;
		//
		Element(Type const t, pos_type p, Font const & f, Change const & ch)
			: type(t), pos(p), endpos(p + 1), font(f), change(ch) {}


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
		/** Break the element in two if possible, so that its width is less
		 * than \param w.
		 * \return an element containing the remainder of the text, or
		 *   an invalid element if nothing happened.
		 * \param w: the desired maximum width
		 * \param force: if true, the string is cut at any place, otherwise it
		 *   respects the row breaking rules of characters.
		 */
		Element splitAt(int w, bool force);
		/** Break the element if possible, so that its width is less
		 * than \param w. Returns true on success. When \param force
		 * is true, the string is cut at any place, otherwise it
		 * respects the row breaking rules of characters.
		 */
		bool breakAt(int w, bool force);

		//
		bool isRTL() const { return font.isVisibleRightToLeft(); }
		// This is true for virtual elements.
		bool isVirtual() const { return type == VIRTUAL; }
		// Invalid element, for error handling
		bool isValid() const { return type !=INVALID; }

		// Returns the position on left side of the element.
		pos_type left_pos() const { return isRTL() ? endpos : pos; };
		// Returns the position on right side of the element.
		pos_type right_pos() const { return isRTL() ? pos : endpos; };

		// The kind of row element
		Type type = INVALID;
		// position of the element in the paragraph
		pos_type pos = 0;
		// first position after the element in the paragraph
		pos_type endpos = 0;
		// The dimension of the chunk (does not contains the
		// separator correction)
		Dimension dim;

		// Non-zero only if element is an inset
		Inset const * inset = nullptr;

		// Only non-null for justified rows
		double extra = 0;

		// Non-empty if element is a string or is virtual
		docstring str;
		//
		Font font;
		//
		Change change;
		// is it possible to add contents to this element?
		bool final = false;

		friend std::ostream & operator<<(std::ostream & os, Element const & row);
	};


	///
	Row() {}

	/**
	 * Helper function: set variable \c var to value \c val, and mark
	 * row as changed is the values were different. This is intended
	 * for use when changing members of the row object.
	 */
	template<class T1, class T2>
	void change(T1 & var, T2 const val) {
		if (var != val)
			changed(true);
		var = val;
	}
	/**
	 * Helper function: set variable \c var to value \c val, and mark
	 * row as changed is the values were different. This is intended
	 * for use when changing members of the row object.
	 * This is the const version, useful for mutable members.
	 */
	template<class T1, class T2>
	void change(T1 & var, T2 const val) const {
		if (var != val)
			changed(true);
		var = val;
	}
	///
	bool changed() const { return changed_; }
	///
	void changed(bool c) const { changed_ = c; }
	///
	bool selection() const;
	/**
	 * Set the selection begin and end and whether the left and/or
	 * right margins are selected.
	 * This is const because we update the selection status only at
	 * draw() time.
	 */
	void setSelectionAndMargins(DocIterator const & beg,
		DocIterator const & end) const;
	/// no selection on this row.
	void clearSelectionAndMargins() const;

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
	Dimension const & dim() const { return dim_; }
	///
	Dimension & dim() { return dim_; }
	///
	int height() const { return dim_.height(); }
	/// The width of the row, including the left margin, but not the right one.
	int width() const { return dim_.wid; }
	///
	int ascent() const { return dim_.asc; }
	///
	int descent() const { return dim_.des; }

	///
	Dimension const & contents_dim() const { return contents_dim_; }
	///
	Dimension & contents_dim() { return contents_dim_; }

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
	/// add element at the end and update width
	void push_back(Element const &);
	/// remove last element and update width
	void pop_back();
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
	double separator = 0;
	/// width of hfills in the label
	double label_hfill = 0;
	/// the left margin position of the row
	int left_margin = 0;
	/// the right margin of the row
	int right_margin = 0;
	///
	mutable pos_type sel_beg = -1;
	///
	mutable pos_type sel_end = -1;
	///
	mutable bool begin_margin_sel = false;
	///
	mutable bool end_margin_sel = false;

private:
	/// Decides whether the margin is selected.
	/**
	  * \param margin_begin
	  * \param beg
	  * \param end
	  */
	bool isMarginSelected(bool left, DocIterator const & beg,
		DocIterator const & end) const;
	/// Set the selection begin and end.
	void setSelection(pos_type sel_beg, pos_type sel_end) const;

	/**
	 * Returns true if a char or string with font \c f and change
	 * type \c ch can be added to the current last element of the
	 * row.
	 */
	bool sameString(Font const & f, Change const & ch) const;

	///
	Elements elements_;

	/// has the Row appearance changed since last drawing?
	mutable bool changed_ = true;
	/// Index of the paragraph that contains this row
	pit_type pit_ = 0;
	/// first pos covered by this row
	pos_type pos_ = 0;
	/// one behind last pos covered by this row
	pos_type end_ = 0;
	// Is there a boundary at the end of the row (display inset...)
	bool right_boundary_ = false;
	// Shall the row be flushed when it is supposed to be justified?
	bool flushed_ = false;
	/// Row dimension.
	Dimension dim_;
	/// Row contents dimension. Does not contain the space above/below row.
	Dimension contents_dim_;
	/// true when this row lives in a right-to-left paragraph
	bool rtl_ = false;
	/// true when a changebar should be drawn in the margin
	bool changebar_ = false;
};


/**
 * Each paragraph is broken up into a number of rows on the screen.
 * This is a list of such on-screen rows, ordered from the top row
 * downwards.
 */
typedef std::vector<Row> RowList;

} // namespace lyx

#endif
