// -*- C++ -*-
/**
 * \file TexRow.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Matthias Ettrich
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

/* Note about debugging options:
 *
 * When compiled in devel mode and run with the option -dbg latex, two ways
 * of debugging TexRow are available:
 *
 * 1. The source view panel prepends the full TexRow information to the LaTeX
 *    output.
 *
 * 2. Clicking on any line in the source view moves the buffer to the location
 *    recognised by TexRow.
 *
 */

#ifndef TEXROW_H
#define TEXROW_H

#include "support/types.h"
#include "support/debug.h"

#include <vector>

namespace lyx {

class LyXErr;
class Cursor;
class CursorSlice;
class DocIterator;
class docstring_list;

/// types for cells and math insets
typedef void const * uid_type;
typedef size_t idx_type;


/// an individual par id/pos <=> row mapping
struct TextEntry { int id; int pos; };

/// an individual math id/cell <=> row mapping
struct MathEntry { uid_type id; idx_type cell; };

/// a container for passing entries around
struct RowEntry {
	bool is_math;// true iff the union is a math
	union {
		struct TextEntry text;
		struct MathEntry math;
	};
};


/// Represents the correspondence between paragraphs and the generated
/// LaTeX file

class TexRow {
public:
	// For each row we store a list of one special TextEntry and several
	// RowEntries. (The order is important.)  We only want one text entry
	// because we do not want to store every position in the lyx file. On the
	// other hand we want to record all math and table cells positions for
	// enough precision. Usually the count of cells is easier to handle.
	class RowEntryList : public std::vector<RowEntry> {
	public:
		RowEntryList() : std::vector<RowEntry>(), text_entry_(-1) {}

		// returns true if the row entry will appear in the row entry list
		bool addEntry(RowEntry const &);

		// the row entry will appear in the row entry list, but it never counts
		// as a proper text entry.
		void forceAddEntry(RowEntry const &);

		// returns the TextEntry or TexRow::text_none if none
		TextEntry getTextEntry() const;

		// returns the first entry, or TexRow::row_none if none
		RowEntry entry() const;

		// appends a row
		void append(RowEntryList const &);

	private:
		size_t text_entry_;
	};

	/// Returns true if RowEntry is devoid of information
	static bool isNone(RowEntry const &);
	static const TextEntry text_none;
	static const RowEntry row_none;

	/// Returns true if TextEntry is devoid of information
	static bool isNone(TextEntry const &);

	/// Converts a CursorSlice into a RowEntry
	static RowEntry rowEntryFromCursorSlice(CursorSlice const & slice);

	/// Encapsulates the paragraph and position for later use
	static RowEntry textEntry(int id, int pos);

	/// Encapsulates a cell and position for later use
	static RowEntry mathEntry(uid_type id, idx_type cell);

	/// true iff same paragraph or math inset
	static bool sameParOrInsetMath(RowEntry const &, RowEntry const &);

	/// computes the distance in pos or cell index
	/// assumes it is the sameParOrInsetMath
	static int comparePos(RowEntry const & entry1, RowEntry const & entry2);

	/// for debugging purposes
	static docstring asString(RowEntry const &);

	///
	TexRow(bool enable = true)
		: current_row_(RowEntryList()), enabled_(enable) {}

	/// Clears structure.  Set enable to false if texrow is not needed, to avoid
	/// computing TexRow when it is going to be immediately discarded.
	void reset(bool enable = true);

	/// Defines the row information for the current line
	/// returns true if this entry will appear on the current row
	bool start(RowEntry entry);

	/// Defines the paragraph and position for the current line
	/// returns true if this entry will appear on the current row
	bool start(int id, int pos);

	/// Defines a cell and position for the current line.  Always appear in the
	/// current row.
	void startMath(uid_type id, idx_type cell);

	/// Defines the paragraph for the current cell-like inset.  Always appears
	/// in the current row like a math cell, but is detached from the normal
	/// text flow. Note: since the cell idx is not recorded it does not work as
	/// well as for math grids; if we were to do that properly we would need to
	/// access the id of the parent Tabular inset from the CursorSlice.
	void forceStart(int id, int pos);

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

	/// Finds the best pair of rows for cursor, taking the selection into
	/// account
	/// returns (-1,-1) if not found.
	std::pair<int,int> rowFromCursor(Cursor const & dit) const;
	
	/// Returns the number of rows contained
	int rows() const { return rowlist_.size(); }

	/// appends texrow. the final line of this is merged with the first line of
	/// texrow.
	void append(TexRow const & texrow);

	/// for debugging purpose
	void prepend(docstring_list &) const;

private:
	typedef std::vector<RowEntryList> RowList;
	///
	class RowListIterator;
	///
	RowListIterator begin() const;
	///
	RowListIterator end() const;
	/// container of id/pos <=> row mapping
	RowList rowlist_;
	/// Entry of current line
	RowEntryList current_row_;
	/// 
	bool enabled_;
};

bool operator==(RowEntry const &, RowEntry const &);

LyXErr & operator<<(LyXErr &, TexRow &);


} // namespace lyx

#endif // TEXROW_H
