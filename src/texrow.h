// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team
 *
 * ====================================================== */

#ifndef TEXROW_H
#define TEXROW_H

#ifdef __GNUG__
#pragma interface
#endif

#include <list>

class LyXParagraph;

// Controls correspondance between paragraphs and the generated LaTeX file
class TexRow {
public:
	///
	TexRow() {
		count = 0;
		lastpar = 0;
		lastpos = -1;
	}

	/// Clears structure
	void reset();

	/// Define what paragraph and position the next row will represent
	void start(LyXParagraph * par, int pos);

	/// Insert node when line is completed
	void newline();

	/// Returns paragraph id and position from a row number
	void getIdFromRow(int row, int & id, int & pos);

	/// Appends another TexRow
	TexRow & operator+= (TexRow const &);

private:
	/// Linked list of items
	struct RowItem {
		///
		RowItem() {
			id = -1;
			pos = -1;
			rownumber = 0;
		}

		///
		int id;
		///
		int pos;
		///
		int rownumber;
	};
	///
	unsigned int count;
	///
	typedef std::list<RowItem> RowList;
	///
	RowList rowlist;
	/// Last paragraph
	LyXParagraph * lastpar;
	/// Last position
	int lastpos;
	
};
#endif
