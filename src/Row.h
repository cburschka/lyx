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
 * An on-screen row of text. A paragraph is broken into a
 * RowList for display. Each Row contains position pointers
 * into the first and last character positions of that row.
 */
class Row {
public:
/**
 * One element of a Row. It has a set of attributes that can be used
 * by other methods that need to parse the Row contents.
 */
	struct Element {
		enum Type {
			STRING_ELT,
			SEPARATOR_ELT,
			INSET_ELT,
			SPACE_ELT
		};

		Element(Type const t) : type(t), pos(0), inset(0), 
					final(false) {}

		//
		bool isLineSeparator() const { return type == SEPARATOR_ELT; }

		// The kind of row element
		Type type;
		// position of the element in the paragraph
		pos_type pos;
		// The dimension of the chunk (only width for strings)
		Dimension dim;

		// Non-zero if element is an inset
		Inset const * inset;

		// Non-empty if element is a string or separator
		docstring str;
		// is it possible to add contents to this element?
		bool final;
		//
		Font font;
		//
		Change change;
	};


	///
	Row();
	///
	bool changed() const { return changed_; }
	///
	void setChanged(bool c) { changed_ = c; }
	///
	void setCrc(size_type crc) const;
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
	void pos(pos_type p);
	///
	pos_type pos() const { return pos_; }
	///
	void endpos(pos_type p);
	///
	pos_type endpos() const { return end_; }
	///
	Dimension const & dimension() const { return dim_; }
	///
	Dimension & dimension() { return dim_; }
	///
	int height() const { return dim_.height(); }
	///
	int width() const { return dim_.wid; }
	///
	int ascent() const { return dim_.asc; }
	///
	int descent() const { return dim_.des; }

	///
	void add(pos_type pos, Inset const * ins, Dimension const & dim);
	///
	void add(pos_type pos, docstring const & s,
		 Font const & f, Change const & ch);
	///
	void add(pos_type pos, char_type const c,
		 Font const & f, Change const & ch);
	///
	void addSeparator(pos_type pos, char_type const c,
			  Font const & f, Change const & ch);
	///
	void addSpace(pos_type pos, int width);
	///
	bool empty() const { return elements_.empty(); }
	///
	Element & back() { return elements_.back(); }
	///
	Element const & back() const { return elements_.back(); }
	/// remove last element
	void pop_back();
	/// remove all row elements
	void clear() { elements_.clear(); }
	/**
	 * remove all elements after last separator and update endpos
	 * if necessary. 
	 * \param keep is the minimum amount of text to keep.
	 */
	void separate_back(pos_type keep);

	/**
	 * If last element of the row is a string, compute its width
	 * and mark it final.
	 */
	void finalizeLast();

	friend std::ostream & operator<<(std::ostream & os, Row const & row);

	/// current debugging only
	void dump(char const * = "") const;

	/// width of a separator (i.e. space)
	double separator;
	/// width of hfills in the label
	double label_hfill;
	/// the x position of the row
	double x;
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
	typedef std::vector<Element> Elements;
	///
	Elements elements_;

	/// has the Row appearance changed since last drawing?
	mutable bool changed_;
	/// CRC of row contents.
	mutable size_type crc_;
	/// first pos covered by this row
	pos_type pos_;
	/// one behind last pos covered by this row
	pos_type end_;
	/// Row dimension.
	Dimension dim_;
};


} // namespace lyx

#endif
