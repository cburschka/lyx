// -*- C++ -*-
/**
 * \file lyxrow.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS
 *
 * Metrics for an on-screen text row.
 */

#ifndef LYXROW_H
#define LYXROW_H

#include "support/types.h"

class Paragraph;

///
class Row {
public:
	///
	Row();

	///
	void par(Paragraph * p);
	///
	Paragraph * par();
	///
	Paragraph * par() const;
	///
	void pos(lyx::pos_type p);
	///
	lyx::pos_type pos() const;
	///
	void fill(int f);
	///
	int fill() const;
	///
	void height(unsigned short h);
	///
	unsigned short height() const;
	///
	void width(unsigned int w);
	///
	unsigned int width() const;
	///
	void ascent_of_text(unsigned short a);
	///
	unsigned short ascent_of_text() const;
	///
	void top_of_text(unsigned int top);
	///
	unsigned int top_of_text() const;
	///
	void baseline(unsigned int b);
	///
	unsigned int baseline() const;
	///
	void next(Row * r);
	///
	Row * next() const;
	///
	void previous(Row * r);
	///
	Row * previous() const;

	/// return the position of the last character in this row
	lyx::pos_type lastPos() const;
	/// return the position of the last normal, printable character in this row
	lyx::pos_type lastPrintablePos() const;
	
	/**
	 * Returns the number of separators.
	 * The separator on the very last column doesnt count.
	 */
	int numberOfSeparators() const;

	/** 
	 * Returns the number of hfills. It works like a LaTeX \hfill:
	 * the hfills at the beginning and at the end are ignored.
	 * This is much more useful than not to ignore!
	 */
	int numberOfHfills() const;

	/// Returns the number of hfills in the manual label. See numberOfHfills().
	int numberOfLabelHfills() const;

	/**
	 * Returns true if a expansion is needed at the given position.
	 * Rules are given by LaTeX
	 */
	bool hfillExpansion(lyx::pos_type pos) const;

private:
	///
	Paragraph * par_;
	///
	lyx::pos_type pos_;
	/** what is missing to a full row can be negative.
	  Needed for hfills, flushright, block etc. */
	mutable int fill_;
	///
	unsigned short height_;
	///
	unsigned int width_;
	/// ascent from baseline including prelude space
	unsigned short ascent_of_text_;
	/// the top of the real text in the row
	unsigned int top_of_text_;
	///
	unsigned int baseline_;
	///
	Row * next_;
	///
	Row * previous_;
};


inline
Paragraph * Row::par()
{
	return par_;
}


inline
Paragraph * Row::par() const
{
	return par_;
}


inline
unsigned short Row::height() const
{
	return height_;
}


inline
Row * Row::next() const
{
	return next_;
}

#endif
