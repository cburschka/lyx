// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *          Copyright (C) 1995-1998 The LyX Team
 *
 *======================================================*/

#ifndef _TEXROW_H
#define _TEXROW_H

#ifdef __GNUG__
#pragma interface
#endif

class LyXParagraph;

// Controls correspondance between paragraphs and the generated LaTeX file
class TexRow {
public:
	///
	TexRow() {
		count = 0;
		next = 0;
		lastpar = 0;
		lastpos = -1;
	}
	///
	~TexRow() {
		reset();
	}

	/// Clears structure
	void reset();

	/// Define what paragraph and position the next row will represent
	void start(LyXParagraph *par, int pos);

	/// Insert node when line is completed
	void newline();

	/// Returns paragraph id and position from a row number
	void getIdFromRow(int row, int &id, int &pos);

	/// Appends another TexRow
	TexRow & operator+=(const TexRow &);

private:
	/// Linked list of items
	struct TexRow_Item {
		///
		TexRow_Item() {
			id = -1;
			pos = -1;
			next = 0;
			rownumber = 0;
		}

		///
		int id;
		///
		int pos;
		///
		int rownumber;
		///
		TexRow_Item *next;
	};
	///
	unsigned int count;
	///
	TexRow_Item *next;
	/// Last paragraph
	LyXParagraph * lastpar;
	/// Last position
	int lastpos;
	
};
#endif
