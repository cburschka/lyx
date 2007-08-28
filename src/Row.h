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
	Row(pos_type pos);
	///
	void pos(pos_type p);
	///
	pos_type pos() const;
	///
	void endpos(pos_type p);
	///
	pos_type endpos() const;
	///
	Dimension const & dimension() const { return dim_; }
	///
	int height() const { return dim_.height(); }
	///
	void width(int w) { dim_.wid = w; }
	///
	int width() const { return dim_.wid; }
	///
	void ascent(int a) { dim_.asc = a; }
	///
	int ascent() const { return dim_.asc; }
	///
	void descent(int d) { dim_.des = d; }
	///
	int descent() const { return dim_.des; }
	/// current debugging only
	void dump(const char * = "") const;

private:
	/// first pos covered by this row
	pos_type pos_;
	/// one behind last pos covered by this row
	pos_type end_;
	/// Row dimension.
	Dimension dim_;
};


class RowMetrics {
public:
	RowMetrics();
	/// width of a separator (i.e. space)
	double separator;
	/// width of hfills in the body
	double hfill;
	/// width of hfills in the label
	double label_hfill;
	/// the x position of the row
	double x;
};



} // namespace lyx

#endif
