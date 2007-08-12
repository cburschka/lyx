// -*- C++ -*-
/**
 * \file TexRow.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Matthias Ettrich
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TEXROW_H
#define TEXROW_H

#include <list>


namespace lyx {


/// Represents the correspondence between paragraphs and the generated
//LaTeX file

class TexRow {
public:
	///
	TexRow() : count(0), lastid(-1), lastpos(-1) {}

	TexRow & operator+= (TexRow const &);

	/// Clears structure
	void reset();

	/// Define what paragraph and position the next row will represent
	void start(int id, int pos);

	/// Insert node when line is completed
	void newline();

	/**
	 * getIdFromRow - find pid and position for a given row
	 * @param row row number to find
	 * @param id set to id if found
	 * @param pos set to paragraph position if found
	 * @return true if found, false otherwise
	 *
	 * If the row could not be found, pos is set to zero and
	 * id is set to -1
	 */
	bool getIdFromRow(int row, int & id, int & pos) const;

	/// Returns the number of rows contained
	int rows() const { return count; }

	/// an individual id/pos <=> row mapping
	class RowItem {
	public:
		RowItem(int id, int pos, int row)
			: id_(id), pos_(pos), rownumber_(row)
		{}

		/// paragraph id
		int id() const { return id_; }
		/// set paragraph position
		void pos(int p) { pos_ = p; }
		/// paragraph position
		int pos() const { return pos_; }
		/// row number
		int rownumber() const { return rownumber_; }
	private:
		RowItem();
		int id_;
		int pos_;
		int rownumber_;
	};
	///
	typedef std::list<RowItem> RowList;
private:
	/// number of lines
	unsigned int count;
	/// container of id/pos <=> row mapping
	RowList rowlist;
	/// Last paragraph
	int lastid;
	/// Last position
	int lastpos;
};


} // namespace lyx

#endif // TEXROW_H
