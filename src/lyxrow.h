// -*- C++ -*-
/**
 * \file lyxrow.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Matthias Ettrich
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 *
 * Metrics for an on-screen text row.
 */

#ifndef LYXROW_H
#define LYXROW_H

#include "ParagraphList.h"
#include "support/types.h"

///
class Row {
public:
	///
	Row();
	///
	Row(ParagraphList::iterator pit, lyx::pos_type pos);
	///
	void par(ParagraphList::iterator pit);
	///
	ParagraphList::iterator par();
	///
	ParagraphList::iterator par() const;
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
	/// return true if this row is the start of a paragraph
	bool isParStart() const;
	/// return the cached y position
	unsigned int y() const;
	/// cache the y position
	void y(unsigned int newy);
private:
	///
	ParagraphList::iterator pit_;
	///
	lyx::pos_type pos_;
	/** what is missing to a full row. Can be negative.
	  Needed for hfills, flushright, block etc. */
	mutable int fill_;
	///
	unsigned short height_;
	///
	unsigned int width_;
	/// cached y position
	unsigned int y_;
	/// ascent from baseline including prelude space
	unsigned short ascent_of_text_;
	/// the top of the real text in the row
	unsigned int top_of_text_;
	///
	unsigned int baseline_;
};

#endif
