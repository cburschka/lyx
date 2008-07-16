// -*- C++ -*-
/**
 * \file InsetTabular.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 * \author André Pönitz
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */


// This is Juergen's rewrite of the tabular (table) support.

// Things to think of when designing the new tabular support:
// - color support (colortbl, color)
// - decimal alignment (dcloumn)
// - custom lines (hhline)
// - rotation
// - multicolumn
// - multirow
// - column styles

// This is what I have written about tabular support in the LyX3-Tasks file:
//
//  o rewrite of table code. Should probably be written as some
//    kind of an inset. [Done]
// o enhance longtable support

// Lgb

#ifndef INSET_TABULAR_H
#define INSET_TABULAR_H

#include "Inset.h"
#include "InsetText.h"
#include "Layout.h"
#include "Length.h"

#include <boost/shared_ptr.hpp>

#include <iosfwd>
#include <vector>

namespace lyx {

class Buffer;
class BufferParams;
class BufferView;
class CompletionList;
class CursorSlice;
class InsetTableCell;
class FuncStatus;
class Lexer;
class Paragraph;

namespace frontend { class Painter; }


class InsetTabular;
class Cursor;
class OutputParams;

//
// A helper struct for tables
//
class Tabular {
public:
	///
	enum Feature {
		///
		APPEND_ROW = 0,
		///
		APPEND_COLUMN,
		///
		DELETE_ROW,
		///
		DELETE_COLUMN,
		///
		COPY_ROW,
		///
		COPY_COLUMN,
		///
		TOGGLE_LINE_TOP,
		///
		TOGGLE_LINE_BOTTOM,
		///
		TOGGLE_LINE_LEFT,
		///
		TOGGLE_LINE_RIGHT,
		///
		ALIGN_LEFT,
		///
		ALIGN_RIGHT,
		///
		ALIGN_CENTER,
		///
		ALIGN_BLOCK,
		///
		VALIGN_TOP,
		///
		VALIGN_BOTTOM,
		///
		VALIGN_MIDDLE,
		///
		M_ALIGN_LEFT,
		///
		M_ALIGN_RIGHT,
		///
		M_ALIGN_CENTER,
		///
		M_VALIGN_TOP,
		///
		M_VALIGN_BOTTOM,
		///
		M_VALIGN_MIDDLE,
		///
		MULTICOLUMN,
		///
		SET_ALL_LINES,
		///
		UNSET_ALL_LINES,
		///
		SET_LONGTABULAR,
		///
		UNSET_LONGTABULAR,
		///
		SET_PWIDTH,
		///
		SET_MPWIDTH,
		///
		SET_ROTATE_TABULAR,
		///
		UNSET_ROTATE_TABULAR,
		///
		TOGGLE_ROTATE_TABULAR,
		///
		SET_ROTATE_CELL,
		///
		UNSET_ROTATE_CELL,
		///
		TOGGLE_ROTATE_CELL,
		///
		SET_USEBOX,
		///
		SET_LTHEAD,
		UNSET_LTHEAD,
		///
		SET_LTFIRSTHEAD,
		UNSET_LTFIRSTHEAD,
		///
		SET_LTFOOT,
		UNSET_LTFOOT,
		///
		SET_LTLASTFOOT,
		UNSET_LTLASTFOOT,
		///
		SET_LTNEWPAGE,
		///
		TOGGLE_LTCAPTION,
		///
		SET_SPECIAL_COLUMN,
		///
		SET_SPECIAL_MULTI,
		///
		SET_BOOKTABS,
		///
		UNSET_BOOKTABS,
		///
		SET_TOP_SPACE,
		///
		SET_BOTTOM_SPACE,
		///
		SET_INTERLINE_SPACE,
		///
		SET_BORDER_LINES,
		///
		LAST_ACTION
	};
	///
	enum {
		///
		CELL_NORMAL = 0,
		///
		CELL_BEGIN_OF_MULTICOLUMN,
		///
		CELL_PART_OF_MULTICOLUMN
	};

	///
	enum VAlignment {
		///
		LYX_VALIGN_TOP = 0,
		///
		LYX_VALIGN_BOTTOM = 1,
		///
		LYX_VALIGN_MIDDLE = 2
	};

	enum BoxType {
		///
		BOX_NONE = 0,
		///
		BOX_PARBOX = 1,
		///
		BOX_MINIPAGE = 2
	};

	class ltType {
	public:
		// constructor
		ltType();
		// we have this header type (is set in the getLT... functions)
		bool set;
		// double borders on top
		bool topDL;
		// double borders on bottom
		bool bottomDL;
		// used for FirstHeader & LastFooter and if this is true
		// all the rows marked as FirstHeader or LastFooter are
		// ignored in the output and it is set to be empty!
		bool empty;
	};

	/// type for row numbers
	typedef size_t row_type;
	/// type for column numbers
	typedef size_t col_type;
	/// type for cell indices
	typedef size_t idx_type;
	/// index indicating an invalid position
	static const idx_type npos = static_cast<idx_type>(-1);

	/// constructor
	Tabular(Buffer const &, col_type columns_arg, row_type rows_arg);

	/// Returns true if there is a topline, returns false if not
	bool topLine(idx_type cell) const;
	/// Returns true if there is a topline, returns false if not
	bool bottomLine(idx_type cell) const;
	/// Returns true if there is a topline, returns false if not
	bool leftLine(idx_type cell) const;
	/// Returns true if there is a topline, returns false if not
	bool rightLine(idx_type cell) const;

	///
	bool topAlreadyDrawn(idx_type cell) const;
	///
	bool leftAlreadyDrawn(idx_type cell) const;
	///
	bool isLastRow(idx_type cell) const;

	/// return space occupied by the second horizontal line and
	/// interline space above row \p row in pixels
	int getAdditionalHeight(row_type row) const;
	///
	int getAdditionalWidth(idx_type cell) const;

	/* returns the maximum over all rows */
	///
	int columnWidth(idx_type cell) const;
	///
	int width() const;
	///
	int height() const;
	///
	int rowAscent(row_type row) const;
	///
	int rowDescent(row_type row) const;
	///
	void setRowAscent(row_type row, int height);
	///
	void setRowDescent(row_type row, int height);
	///
	void setCellWidth(idx_type cell, int new_width);
	///
	void setAllLines(idx_type cell, bool line);
	///
	void setTopLine(idx_type cell, bool line);
	///
	void setBottomLine(idx_type cell, bool line);
	///
	void setLeftLine(idx_type cell, bool line);
	///
	void setRightLine(idx_type cell, bool line);
	///
	bool rowTopLine(row_type row) const;
	///
	bool rowBottomLine(row_type row) const;
	///
	bool columnLeftLine(col_type column) const;
	///
	bool columnRightLine(col_type column) const;

	void setAlignment(idx_type cell, LyXAlignment align,
			  bool onlycolumn = false);
	///
	void setVAlignment(idx_type cell, VAlignment align,
			   bool onlycolumn = false);
	///
	void setColumnPWidth(Cursor &, idx_type, Length const &);
	///
	bool setMColumnPWidth(Cursor &, idx_type, Length const &);
	///
	void setAlignSpecial(idx_type cell, docstring const & special,
			     Feature what);
	///
	LyXAlignment getAlignment(idx_type cell,
				  bool onlycolumn = false) const;
	///
	VAlignment getVAlignment(idx_type cell,
				 bool onlycolumn = false) const;
	///
	Length const getPWidth(idx_type cell) const;
	///
	int cellWidth(idx_type cell) const;
	///
	int getBeginningOfTextInCell(idx_type cell) const;
	///
	void appendRow(idx_type cell);
	///
	void deleteRow(row_type row);
	///
	void copyRow(row_type);
	///
	void appendColumn(idx_type cell);
	///
	void deleteColumn(col_type column);
	///
	void copyColumn(col_type);
	///
	bool isFirstCellInRow(idx_type cell) const;
	///
	idx_type getFirstCellInRow(row_type row) const;
	///
	bool isLastCellInRow(idx_type cell) const;
	///
	idx_type getLastCellInRow(row_type row) const;
	///
	idx_type numberOfCellsInRow(idx_type cell) const;
	///
	void write(std::ostream &) const;
	///
	void read(Lexer &);
	///
	int latex(odocstream &, OutputParams const &) const;
	//
	int docbook(odocstream & os, OutputParams const &) const;
	///
	void plaintext(odocstream &,
		       OutputParams const & runparams, int const depth,
		       bool onlydata, char_type delim) const;
	///
	bool isMultiColumn(idx_type cell) const;
	///
	bool isMultiColumnReal(idx_type cell) const;
	///
	void setMultiColumn(idx_type cell, idx_type number);
	///
	idx_type unsetMultiColumn(idx_type cell); // returns number of new cells
	///
	bool isPartOfMultiColumn(row_type row, col_type column) const;
	///
	row_type cellRow(idx_type cell) const;
	///
	col_type cellColumn(idx_type cell) const;
	///
	col_type cellRightColumn(idx_type cell) const;
	///
	void setRotateCell(idx_type cell, bool);
	///
	bool getRotateCell(idx_type cell) const;
	///
	bool needRotating() const;
	///
	bool isLastCell(idx_type cell) const;
	///
	idx_type cellAbove(idx_type cell) const;
	///
	idx_type cellBelow(idx_type cell) const;
	///
	idx_type cellIndex(row_type row, col_type column) const;
	///
	void setUsebox(idx_type cell, BoxType);
	///
	BoxType getUsebox(idx_type cell) const;
	//
	// Long Tabular Options support functions
	///
	bool checkLTType(row_type row, ltType const &) const;
	///
	void setLTHead(row_type row, bool flag, ltType const &, bool first);
	///
	bool getRowOfLTHead(row_type row, ltType &) const;
	///
	bool getRowOfLTFirstHead(row_type row, ltType &) const;
	///
	void setLTFoot(row_type row, bool flag, ltType const &, bool last);
	///
	bool getRowOfLTFoot(row_type row, ltType &) const;
	///
	bool getRowOfLTLastFoot(row_type row, ltType &) const;
	///
	void setLTNewPage(row_type row, bool what);
	///
	bool getLTNewPage(row_type row) const;
	///
	idx_type setLTCaption(row_type row, bool what);
	///
	bool ltCaption(row_type row) const;
	///
	bool haveLTHead() const;
	///
	bool haveLTFirstHead() const;
	///
	bool haveLTFoot() const;
	///
	bool haveLTLastFoot() const;
	///
	// end longtable support
	///
	boost::shared_ptr<InsetTableCell> cellInset(idx_type cell) const;
	///
	boost::shared_ptr<InsetTableCell> cellInset(row_type row,
						  col_type column) const;
	///
	void setCellInset(row_type row, col_type column,
			  boost::shared_ptr<InsetTableCell>) const;
	/// Search for \param inset in the tabular, with the
	///
	void validate(LaTeXFeatures &) const;
//private:
  // FIXME Now that cells have an InsetTableCell as their insets, rather
  // than an InsetText, it'd be possible to reverse the relationship here,
  // so that cell_vector was a vector<InsetTableCell> rather than a 
  // vector<CellData>, and an InsetTableCell had a CellData as a member,
  // or perhaps just had its members as members.
	///
	class CellData {
	public:
		///
		CellData(Buffer const &);
		///
		CellData(CellData const &);
		///
		CellData & operator=(CellData);
		///
		void swap(CellData & rhs);
		///
		idx_type cellno;
		///
		int width;
		///
		int multicolumn;
		///
		LyXAlignment alignment;
		///
		VAlignment valignment;
		///
		bool top_line;
		///
		bool bottom_line;
		///
		bool left_line;
		///
		bool right_line;
		///
		BoxType usebox;
		///
		bool rotate;
		///
		docstring align_special;
		///
		Length p_width; // this is only set for multicolumn!!!
		///
		boost::shared_ptr<InsetTableCell> inset;
	};
	CellData & cellInfo(idx_type cell) const;
	///
	typedef std::vector<CellData> cell_vector;
	///
	typedef std::vector<cell_vector> cell_vvector;

	///
	class RowData {
	public:
		///
		RowData();
		///
		int ascent;
		///
		int descent;
		/// Extra space between the top line and this row
		Length top_space;
		/// Ignore top_space if true and use the default top space
		bool top_space_default;
		/// Extra space between this row and the bottom line
		Length bottom_space;
		/// Ignore bottom_space if true and use the default bottom space
		bool bottom_space_default;
		/// Extra space between the bottom line and the next top line
		Length interline_space;
		/// Ignore interline_space if true and use the default interline space
		bool interline_space_default;
		/// This are for longtabulars only
		/// a row of endhead
		bool endhead;
		/// a row of endfirsthead
		bool endfirsthead;
		/// a row of endfoot
		bool endfoot;
		/// row of endlastfoot
		bool endlastfoot;
		/// row for a newpage
		bool newpage;
		/// caption
		bool caption;
	};
	///
	typedef std::vector<RowData> row_vector;

	///
	class ColumnData {
		public:
		///
		ColumnData();
		///
		LyXAlignment alignment;
		///
		VAlignment valignment;
		///
		int width;
		///
		Length p_width;
		///
		docstring align_special;
	};
	///
	typedef std::vector<ColumnData> column_vector;

	///
	idx_type numberofcells;
	///
	std::vector<row_type> rowofcell;
	///
	std::vector<col_type> columnofcell;
	///
	row_vector row_info;
	///
	column_vector column_info;
	///
	mutable cell_vvector cell_info;
	///
	bool use_booktabs;
	///
	bool rotate;
	//
	// for long tabulars
	//
	bool is_long_tabular;
	/// endhead data
	ltType endhead;
	/// endfirsthead data
	ltType endfirsthead;
	/// endfoot data
	ltType endfoot;
	/// endlastfoot data
	ltType endlastfoot;

	///
	void init(Buffer const &, row_type rows_arg,
		  col_type columns_arg);
	///
	void updateIndexes();
	/// return true of update is needed
	bool updateColumnWidths();
	///
	idx_type columnSpan(idx_type cell) const;
	///
	BoxType useParbox(idx_type cell) const;
	///
	// helper function for Latex returns number of newlines
	///
	int TeXTopHLine(odocstream &, row_type row) const;
	///
	int TeXBottomHLine(odocstream &, row_type row) const;
	///
	int TeXCellPreamble(odocstream &, idx_type cell, bool & ismulticol) const;
	///
	int TeXCellPostamble(odocstream &, idx_type cell, bool ismulticol) const;
	///
	int TeXLongtableHeaderFooter(odocstream &, OutputParams const &) const;
	///
	bool isValidRow(row_type const row) const;
	///
	int TeXRow(odocstream &, row_type const row,
		   OutputParams const &) const;
	///
	// helper functions for plain text
	///
	bool plaintextTopHLine(odocstream &, row_type row,
			       std::vector<unsigned int> const &) const;
	///
	bool plaintextBottomHLine(odocstream &, row_type row,
				  std::vector<unsigned int> const &) const;
	///
	void plaintextPrintCell(odocstream &,
				OutputParams const &,
				idx_type cell, row_type row, col_type column,
				std::vector<unsigned int> const &,
				bool onlydata) const;
	/// auxiliary function for docbook
	int docbookRow(odocstream & os, row_type, OutputParams const &) const;

	/// change associated Buffer
	void setBuffer(Buffer const & buffer) { buffer_ = &buffer; }
	/// retrieve associated Buffer
	Buffer const & buffer() const { return *buffer_; }

private:
	Buffer const * buffer_;

}; // Tabular


///
class InsetTableCell : public InsetText
{
public:
	///
	InsetTableCell(Buffer const & buf);
	///
	InsetCode lyxCode() const { return CELL_CODE; }
	///
	Inset * clone() { return new InsetTableCell(*this); }
	///
	virtual bool usePlainLayout() const { return true; }
	/// 
	virtual bool forcePlainLayout(idx_type = 0) const;
	/// 
	virtual bool allowParagraphCustomization(idx_type = 0) const;
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & status) const;
	///
	virtual bool neverIndent() { return true; }
	///
	void toggleFixedWidth(bool fw) { isFixedWidth = fw; }
private:
	/// unimplemented
	InsetTableCell();
	/// unimplemented
	void operator=(InsetTableCell const &);
	///
	bool isFixedWidth;
};


class InsetTabular : public Inset
{
public:
	///
	InsetTabular(Buffer const &, row_type rows = 1,
		     col_type columns = 1);
	///
	~InsetTabular();
	///
	static void string2params(std::string const &, InsetTabular &);
	///
	static std::string params2string(InsetTabular const &);
	///
	void read(Lexer &);
	///
	void write(std::ostream &) const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void drawSelection(PainterInfo & pi, int x, int y) const;
	///
	docstring editMessage() const;
	///
	EDITABLE editable() const { return HIGHLY_EDITABLE; }
	///
	bool insetAllowed(InsetCode code) const;
	///
	bool allowSpellCheck() const { return true; }
	///
	bool canTrackChanges() const { return true; }
	/** returns true if, when outputing LaTeX, font changes should
	    be closed before generating this inset. This is needed for
	    insets that may contain several paragraphs */
	bool noFontChange() const { return true; }
	///
	DisplayType display() const { return tabular.is_long_tabular ? AlignCenter : Inline; }
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	InsetCode lyxCode() const { return TABULAR_CODE; }
	///
	docstring contextMenu(BufferView const & bv, int x, int y) const;
	/// get offset of this cursor slice relative to our upper left corner
	void cursorPos(BufferView const & bv, CursorSlice const & sl,
		bool boundary, int & x, int & y) const;
	///
	bool tabularFeatures(Cursor & cur, std::string const & what);
	///
	void tabularFeatures(Cursor & cur, Tabular::Feature feature,
			     std::string const & val = std::string());
	///
	void openLayoutDialog(BufferView *) const;
	///
	bool showInsetDialog(BufferView *) const;
	/// number of cells
	size_t nargs() const { return tabular.numberofcells; }
	///
	boost::shared_ptr<InsetTableCell const> cell(idx_type) const;
	///
	boost::shared_ptr<InsetTableCell> cell(idx_type);
	///
	Text * getText(int) const;

	/// set the change for the entire inset
	void setChange(Change const & change);
	/// accept the changes within the inset
	void acceptChanges(BufferParams const & bparams);
	/// reject the changes within the inset
	void rejectChanges(BufferParams const & bparams);

	// this should return true if we have a "normal" cell, otherwise false.
	// "normal" means without width set!
	/// should all paragraphs be output with "Standard" layout?
	virtual bool allowParagraphCustomization(idx_type cell = 0) const;
	///
	virtual bool forcePlainLayout(idx_type cell = 0) const;
	///
	virtual bool usePlainLayout() { return true; }
	///
	void addPreview(graphics::PreviewLoader &) const;

	/// lock cell with given index
	void edit(Cursor & cur, bool front, EntryDirection entry_from);
	/// get table row from x coordinate
	int rowFromY(Cursor & cur, int y) const;
	/// get table column from y coordinate
	int columnFromX(Cursor & cur, int x) const;
	///
	Inset * editXY(Cursor & cur, int x, int y);
	/// can we go further down on mouse click?
	bool descendable() const { return true; }
	// Update the counters of this inset and of its contents
	void updateLabels(ParIterator const &);

	///
	bool completionSupported(Cursor const &) const;
	///
	bool inlineCompletionSupported(Cursor const & cur) const;
	///
	bool automaticInlineCompletion() const;
	///
	bool automaticPopupCompletion() const;
	///
	bool showCompletionCursor() const;
	///
	CompletionList const * createCompletionList(Cursor const & cur) const;
	///
	docstring completionPrefix(Cursor const & cur) const;
	///
	bool insertCompletion(Cursor & cur, docstring const & s, bool finished);
	///
	void completionPosAndDim(Cursor const &, int & x, int & y, Dimension & dim) const;

	///
	virtual InsetTabular * asInsetTabular() { return this; }
	///
	virtual InsetTabular const * asInsetTabular() const { return this; }
	///
	bool isRightToLeft(Cursor & cur) const;

	//
	// Public structures and variables
	///
	mutable Tabular tabular;

private:
	///
	InsetTabular(InsetTabular const &);
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;
	///
	int scroll() const { return scx_; }
	///
	Inset * clone() const { return new InsetTabular(*this); }

	///
	void drawCellLines(frontend::Painter &, int x, int y, row_type row,
			   idx_type cell, bool erased) const;
	///
	void setCursorFromCoordinates(Cursor & cur, int x, int y) const;

	///
	void moveNextCell(Cursor & cur, 
				EntryDirection entry_from = ENTRY_DIRECTION_IGNORE);
	///
	void movePrevCell(Cursor & cur,
				EntryDirection entry_from = ENTRY_DIRECTION_IGNORE);
	///
	int cellXPos(idx_type cell) const;
	///
	void resetPos(Cursor & cur) const;
	///
	void removeTabularRow();
	///
	bool copySelection(Cursor & cur);
	///
	bool pasteClipboard(Cursor & cur);
	///
	void cutSelection(Cursor & cur);
	///
	void getSelection(Cursor & cur, row_type & rs, row_type & re,
			  col_type & cs, col_type & ce) const;
	///
	bool insertPlaintextString(BufferView &, docstring const & buf, bool usePaste);
	/// are we operating on several cells?
	bool tablemode(Cursor & cur) const;

	/// return the "Manhattan distance" to nearest corner
	int dist(BufferView &, idx_type cell, int x, int y) const;
	/// return the cell nearest to x, y
	idx_type getNearestCell(BufferView &, int x, int y) const;

	/// test the rotation state of the give cell range.
	bool oneCellHasRotationState(bool rotated,
				row_type row_start, row_type row_end,
				col_type col_start, col_type col_end) const;
	///
	mutable idx_type first_visible_cell;
	///
	mutable int scx_;
	/// true when selecting rows with the mouse
	bool rowselect_;
	/// true when selecting columns with the mouse
	bool colselect_;
};

std::string const featureAsString(Tabular::Feature feature);

} // namespace lyx

#endif // INSET_TABULAR_H
