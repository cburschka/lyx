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

#include "support/debug.h"

#include <vector>

namespace lyx {

class LyXErr;
class DocIterator;

/// Represents the correspondence between paragraphs and the generated
/// LaTeX file

class TexRow {
public:
	///
	TexRow(bool enable = true)
		: lastid(-1), lastpos(-1), started(false), enabled_(enable) {}

	/// Clears structure
	/// TexRow is often computed to be immediately discarded. Set enable to
	/// false if texrow is not needed
	void reset(bool enable = true);

	/// Define what paragraph and position the next row will represent
	void start(int id, int pos);

	/// Insert node when line is completed
	void newline();

	/// Insert multiple nodes when zero or more lines are completed
	void newlines(int num_lines);

	/// Call when code generation is complete
	void finalize();

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

	/// Finds the best pair of rows for dit
	/// returns (-1,-1) if not found.
	std::pair<int,int> rowFromDocIterator(DocIterator const & dit) const;
	
	/// Returns the number of rows contained
	int rows() const { return rowlist.size(); }

	/// an individual id/pos <=> row mapping
	class RowItem {
	public:
		RowItem(int id, int pos)
			: id_(id), pos_(pos)
		{}

		/// paragraph id
		int id() const { return id_; }
		/// set paragraph position
		void pos(int p) { pos_ = p; }
		/// paragraph position
		int pos() const { return pos_; }
	private:
		RowItem();
		int id_;
		int pos_;
	};
	///
	typedef std::vector<RowItem> RowList;
private:
	/// container of id/pos <=> row mapping
	RowList rowlist;
	/// Last paragraph
	int lastid;
	/// Last position
	int lastpos;
	/// Is id/pos already registered for current row?
	bool started;
	/// 
	bool enabled_;
};

LyXErr & operator<<(LyXErr &, TexRow &);


} // namespace lyx

#endif // TEXROW_H
