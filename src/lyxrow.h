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

///
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
	void end(lyx::pos_type p);
	///
	lyx::pos_type end() const;
	///
	void fill(int f);
	///
	int fill() const;
	///
	void height(unsigned int h) { height_ = h; }
	///
	unsigned int height() const { return height_; }
	///
	void width(unsigned int w);
	///
	unsigned int width() const;
	///
	void ascent_of_text(unsigned int a);
	///
	unsigned int ascent_of_text() const;
	///
	void top_of_text(unsigned int top);
	///
	unsigned int top_of_text() const;
	///
	void baseline(unsigned int b);
	///
	unsigned int baseline() const;
	/// return true if this row is the start of a paragraph
	bool isParStart() const;
	/// return the cached y position
	unsigned int y_offset() const { return y_offset_; }
	/// cache the y position
	void y_offset(unsigned int newy) { y_offset_ = newy; }
	///
	float x() const;
	///
	void x(float);
	///
	float fill_separator() const;
	///
	void fill_separator(float);
	///
	float fill_hfill() const;
	///
	void fill_hfill(float);
	///
	float fill_label_hfill() const;
	///
	void fill_label_hfill(float);
	/// current debugging only
	void dump(const char * = "") const;
private:
	/// first pos covered by this row
	lyx::pos_type pos_;
	/// one behind last pos covered by this row
	lyx::pos_type end_;
	/** what is missing to a full row. Can be negative.
	  Needed for hfills, flushright, block etc. */
	mutable int fill_;
	///
	unsigned int height_;
	///
	unsigned int width_;
	/// cached y position
	unsigned int y_offset_;
	/// ascent from baseline including prelude space
	unsigned short ascent_of_text_;
	/// the top of the real text in the row
	unsigned int top_of_text_;
	///
	unsigned int baseline_;
	/// offet from left border
	float x_;
	///
	float fill_separator_;
	///
	float fill_hfill_;
	///
	float fill_label_hfill_;
};

#endif
