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

#include "support/types.h"

#include "Dimension.h"


namespace lyx {

class DocIterator;

/**
 * An on-screen row of text. A paragraph is broken into a
 * RowList for display. Each Row contains position pointers
 * into the first and last character positions of that row.
 */
class Row {
public:
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
