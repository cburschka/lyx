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
	TexRow() : count(0), lastpar(0), lastpos(-1) {}

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

	/// Returns the number of rows in this texrow
	int rows() const { return count; }

private:
	/// Linked list of items
	class RowItem {
	public:
		///
		RowItem() : id_(-1), pos_(-1), rownumber_(0) {}
		///
		void id(int i) {
			id_ = i;
		}
		///
		int id() const {
			return id_;
		}
		///
		void pos(int p) {
			pos_ = p;
		}
		///
		int pos() const {
			return pos_;
		}
		///
		void rownumber(int r) {
			rownumber_ = r;
		}
		///
		int rownumber() const {
			return rownumber_;
		}
	private:
		///
		int id_;
		///
		int pos_;
		///
		int rownumber_;
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
