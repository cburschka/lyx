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

#include "support/docstring.h"
#include "support/types.h"

#include <vector>

namespace lyx {

class Buffer;
class Cursor;
class CursorSlice;
class DocIterator;
class docstring_list;
class FuncRequest;


/// Represents the correspondence between paragraphs and the generated
/// LaTeX file

class TexRow {
public:
	/// We begin with defining the types of row information we are tracking
	///

	/// type of row entries
	enum RowType {
		text_entry,
		math_entry,
		begin_document
	};

	/// an individual par id/pos <=> row mapping
	struct TextEntry { int id; pos_type pos; };

	/// an individual math id/cell <=> row mapping
	struct MathEntry { uid_type id; idx_type cell; };

	/// a container for passing entries around
	struct RowEntry {
		RowType type;
		union {
			struct TextEntry text;// iff the type is text_entry
			struct MathEntry math;// iff the type is row_entry
			struct {} begindocument;// iff the type is begin_document
		};
	};

	/// Encapsulates the paragraph and position for later use
	static RowEntry textEntry(int id, pos_type pos);
	/// Encapsulates a cell and position for later use
	static RowEntry mathEntry(uid_type id, idx_type cell);
	/// Denotes the beginning of the document
	static RowEntry beginDocument();

	/// Converts a CursorSlice into a RowEntry
	static RowEntry rowEntryFromCursorSlice(CursorSlice const & slice);

	static const TextEntry text_none;
	static const RowEntry row_none;
	/// Returns true if RowEntry is devoid of information
	static bool isNone(RowEntry entry);
	/// Returns true if TextEntry is devoid of information
	static bool isNone(TextEntry entry);

private:
	/// id/pos correspondence for a single row
	class RowEntryList;

	/// container of id/pos <=> row mapping
	/// invariant: in any enabled_ TexRow, rowlist_ will contain at least one
	/// Row (the current row)
	typedef std::vector<RowEntryList> RowList;
	///
	RowList rowlist_;
	///
	RowEntryList & currentRow();

	///
	class RowListIterator;
	///
	RowListIterator begin() const;
	///
	RowListIterator end() const;
public:
	///
	TexRow();

	/// Copy can be expensive and is not usually useful for TexRow.
	/// Force explicit copy, prefer move instead. This also prevents
	/// move()s from being converted into copy silently.
	explicit TexRow(TexRow const & other) = default;
	TexRow(TexRow && other) = default;
	TexRow & operator=(TexRow const & other) = default;
	TexRow & operator=(TexRow && other) = default;

	/// Clears structure.
	void reset();

	/// for debugging purposes
	static docstring asString(RowEntry entry);

	/// Defines the row information for the current line
	/// returns true if this entry will appear on the current row
	bool start(RowEntry entry);
	/// Defines the paragraph and position for the current line
	/// returns true if this entry will appear on the current row
	bool start(int id, pos_type pos);
	/// Defines a cell and position for the current line.  Always appear in the
	/// current row.
	void startMath(uid_type id, idx_type cell);
	/// Defines the paragraph for the current cell-like inset.  Always appears
	/// in the current row like a math cell, but is detached from the normal
	/// text flow. Note: since the cell idx is not recorded it does not work as
	/// well as for math grids; if we were to do that properly we would need to
	/// access the id of the parent Tabular inset from the CursorSlice.
	void forceStart(int id, pos_type pos);

	/// Insert node when line is completed
	void newline();
	/// Insert multiple nodes when zero or more lines are completed
	void newlines(size_t num_lines);

	/**
	 * getEntriesFromRow - find pids and position for a given row
	 * This is the main algorithm behind reverse-search.
	 * @param row number to find
	 * @return a pair of TextEntry denoting the start and end of the position.
	 * The TextEntry values can be isNone(). If no row is found then the first
	 * value isNone().
	 */
	std::pair<TextEntry,TextEntry> getEntriesFromRow(int row) const;

	/**
	 * getDocIteratorFromEntries - find pids and positions for a given row
	 * @param buffer where to look
	 * @return a pair of DocIterators denoting the start and end of the
	 * position.  The DocIterators can be invalid.  The starting DocIterator
	 * being invalid means that no location was found.  Note: there is no
	 * guarantee that the DocIterators are in the same inset or even at the
	 * same depth.
	 */
	static std::pair<DocIterator, DocIterator> getDocIteratorsFromEntries(
	    TextEntry start,
	    TextEntry end,
	    Buffer const & buf);

	// A FuncRequest to select from start to end
	static FuncRequest goToFunc(TextEntry start, TextEntry end);
	// A FuncRequest to select a row
	FuncRequest goToFuncFromRow(int const row) const;

	/**
	 * getDocIteratorFromRow - find pids and positions for a given row
	 * @param row number to find
	 * @param buffer where to look
	 * @return a pair of DocIterators as above.
	 */
	std::pair<DocIterator, DocIterator> getDocIteratorsFromRow(
	    int row,
	    Buffer const & buf) const;

	/// Finds the best pair of rows for dit
	/// returns (-1,-1) if not found.
	/// This is the main algorithm behind forward-search.
	std::pair<int,int> rowFromDocIterator(DocIterator const & dit) const;

	/// Finds the best pair of rows for cursor, taking the selection into
	/// account
	/// returns (-1,-1) if not found.
	std::pair<int,int> rowFromCursor(Cursor const & dit) const;

	/// Returns the number of rows contained
	size_t rows() const;
	/// Fill or trim to reach the row count \param r
	void setRows(size_t r);

	/// appends texrow. the final line of this is merged with the first line of
	/// texrow.
	void append(TexRow texrow);

	/// for debugging purpose
	void prepend(docstring_list &) const;

private:
	/// true iff same paragraph or math inset or begin_document
	static bool sameParOrInsetMath(RowEntry entry1, RowEntry entry2);
	/// computes the distance in pos or cell index
	/// assumes it is the sameParOrInsetMath
	static int comparePos(RowEntry entry1, RowEntry entry2);

};


/// TexString : dumb struct to pass around docstrings with TexRow information.
/// They are best created using otexstringstream.
/// They can be output to otexrowstreams and otexstreams.
/// A valid TexString has as many newlines in str as in texrow. Be careful not
/// to introduce a mismatch between the line and the row counts, as this will
/// assert in devel mode when outputting to a otexstream.
struct TexString {
	///
	docstring str;
	///
	TexRow texrow;
	/// Copy can be expensive and is not usually useful for TexString.
	/// Force explicit copy, prefer move instead. This also prevents
	/// move()s from being converted into copy silently.
	explicit TexString(TexString const &) = default;
	TexString(TexString && other) = default;
	TexString & operator=(TexString const & other) = default;
	TexString & operator=(TexString && other) = default;
	/// Empty TexString
	TexString() = default;
	/// Texstring containing str and TexRow with enough lines which are empty
	explicit TexString(docstring str);
	/// Texstring containing str and texrow. Must be valid.
	TexString(docstring str, TexRow texrow);
	/// Ensure that the string and the TexRow have as many newlines.
	void validate();
};


// Standard container needs a complete type
class TexRow::RowEntryList {
	// For each row we store a list of one special TextEntry and several
	// RowEntries. (The order is important.)  We only want one text entry
	// because we do not want to store every position in the lyx file. On the
	// other hand we want to record all math and table cells positions for
	// enough precision. Usually the count of cells is easier to handle.
	// The RowEntries are used for forward-search and the code preview pane.
	std::vector<RowEntry> v_;
	// The TextEntry is currently used for reverse-search and the error
	// reporting dialog. Once the latter are adapted to rely on the more precise
	// RowEntries above, it can be removed.
	TextEntry text_entry_;

public:
	typedef std::vector<RowEntry>::iterator iterator;
	iterator begin() { return v_.begin(); }
	iterator end() { return v_.end(); }
	///
	typedef std::vector<RowEntry>::const_iterator const_iterator;
	const_iterator begin() const { return v_.cbegin(); }
	const_iterator end() const { return v_.cend(); }
	///
	RowEntryList() : text_entry_(TexRow::text_none) {}

	// returns true if the row entry will appear in the row entry list
	bool addEntry(RowEntry entry);

	// the row entry will appear in the row entry list, but it never counts
	// as a proper text entry.
	void forceAddEntry(RowEntry entry);

	// returns the TextEntry or TexRow::text_none if none
	TextEntry getTextEntry() const;

	// appends a row
	void append(RowEntryList row);
};


bool operator==(TexRow::RowEntry entry1, TexRow::RowEntry entry2);


} // namespace lyx

#endif // TEXROW_H
