// -*- C++ -*-
/**
 * \file lyxrow.h
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

#ifndef LYXROW_H
#define LYXROW_H

#include "support/types.h"

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
	Row(lyx::pos_type pos);
	///
	void pos(lyx::pos_type p);
	///
	lyx::pos_type pos() const;
	///
	void endpos(lyx::pos_type p);
	///
	lyx::pos_type endpos() const;
	///
	int height() const { return ascent_ + descent_; }
	///
	void width(int w);
	///
	int width() const;
	///
	void ascent(int b);
	///
	int ascent() const;
	///
	void descent(int b) { descent_ = b; }
	///
	int descent() const { return descent_; }
	/// current debugging only
	void dump(const char * = "") const;

private:
	/// first pos covered by this row
	lyx::pos_type pos_;
	/// one behind last pos covered by this row
	lyx::pos_type end_;
	///
	int ascent_;
	///
	int descent_;
	///
	int width_;
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


#endif
