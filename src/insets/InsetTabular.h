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
 * \author Edwin Leuven
 * \author Uwe Stöhr
 * \author Scott Kostyshak
 *
 * Full author contact details are available in file CREDITS.
 */

// Things to think of when designing the new tabular support:
// - color support (colortbl, color)
// - decimal alignment (dcloumn)
// - custom lines (hhline)
// - column styles

#ifndef INSET_TABULAR_H
#define INSET_TABULAR_H

#include "Changes.h"
#include "InsetText.h"

#include "support/Length.h"
#include "support/types.h"

#include <climits>
#include <iosfwd>
#include <memory>
#include <vector>


namespace lyx {

class Buffer;
class BufferView;
class CompletionList;
class Cursor;
class CursorSlice;
class FuncStatus;
class Lexer;
class OutputParams;
class XMLStream;


///
class InsetTableCell : public InsetText
{
public:
	///
	explicit InsetTableCell(Buffer * buf);
	/// We need this since generation of the default is deprecated
	/// (since we declare the assignment constuctor below).
	InsetTableCell(InsetTableCell const & in) = default;
	///
	InsetCode lyxCode() const override { return CELL_CODE; }
	///
	Inset * clone() const override { return new InsetTableCell(*this); }
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & status) const override;
	///
	void toggleFixedWidth(bool fw) { isFixedWidth = fw; }
	///
	void toggleVarWidth(bool vw) { isVarwidth = vw; }
	///
	void toggleMultiCol(bool m) { isMultiColumn = m; }
	///
	void toggleMultiRow(bool m) { isMultiRow = m; }
	///
	void setContentAlignment(LyXAlignment al) {contentAlign = al; }
	/// writes the contents of the cell as a string, optionally
	/// descending into insets
	docstring asString(bool intoInsets = true);
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype, TocBackend & backend) const override;
	///
	void metrics(MetricsInfo &, Dimension &) const override;
	/// Needs to be same as InsetTabular
	bool inheritFont() const override { return false; }
	/// Can the cell contain several paragraphs?
	bool allowMultiPar() const override { return !isMultiRow && (!isMultiColumn || isFixedWidth); }
	///
	bool canPaintChange(BufferView const &) const override { return false; }
	/// This assures we never output \maketitle in table cells
	bool isInTitle() const override { return true; }
private:
	///
	InsetTableCell() = delete;
	///
	void operator=(InsetTableCell const &) = delete;
	// FIXME
	// These booleans are supposed to track whether the cell has had its
	// width explicitly set and whether it is part of a multicolumn, respectively.
	// We need to know this to determine whether
	// layout changes and paragraph customization are allowed---that is,
	// we need it in forcePlainLayout() and allowParagraphCustomization().
	// Unfortunately, that information is not readily available in
	// InsetTableCell. In the case of multicolumn cells, it is present
	// in CellData, and so would be available here if CellData were to
	// become a member of InsetTableCell. But in the other case, it isn't
	// even available there, but is held in Tabular::ColumnData.
	// So, the present solution uses this boolean to track the information
	// we need to track, and tries to keep it updated. This is not ideal,
	// but the other solutions are no better. These are:
	// (i)  Keep a pointer in InsetTableCell to the table;
	// (ii) Find the table by iterating over the Buffer's insets.
	// Solution (i) raises the problem of updating the pointer when an
	// InsetTableCell is copied, and we'd therefore need a copy constructor
	// in InsetTabular and then in Tabular, which seems messy, given how
	// complicated those classes are. Solution (ii) involves a lot of
	// iterating, since this information is needed quite often, and so may
	// be quite slow.
	// So, well, if someone can do better, please do!
	// --rkh
	///
	bool isFixedWidth;
	///
	bool isVarwidth;
	///
	bool isMultiColumn;
	///
	bool isMultiRow;
	// FIXME: Here the thoughts from the comment above also apply.
	///
	LyXAlignment contentAlign;
	/// should paragraph indentation be omitted in any case?
	bool neverIndent() const override { return true; }
	///
	LyXAlignment contentAlignment() const override { return contentAlign; }
	///
	bool usePlainLayout() const override { return true; }
	///
	bool allowParagraphCustomization(idx_type = 0) const override;
	///
	bool forceLocalFontSwitch() const override;
	/// Is the width forced to some value?
	bool hasFixedWidth() const override { return isFixedWidth; }
};


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
		MOVE_COLUMN_RIGHT,
		///
		MOVE_COLUMN_LEFT,
		///
		MOVE_ROW_DOWN,
		///
		MOVE_ROW_UP,
		///
		SET_LINE_TOP,
		///
		SET_LINE_BOTTOM,
		///
		SET_LINE_LEFT,
		///
		SET_LINE_RIGHT,
		///FIXME: remove
		TOGGLE_LINE_TOP,
		///FIXME: remove
		TOGGLE_LINE_BOTTOM,
		///FIXME: remove
		TOGGLE_LINE_LEFT,
		///FIXME: remove
		TOGGLE_LINE_RIGHT,
		///
		SET_LTRIM_TOP,
		///
		SET_RTRIM_TOP,
		///
		SET_LTRIM_BOTTOM,
		///
		SET_RTRIM_BOTTOM,
		///
		TOGGLE_LTRIM_TOP,
		///
		TOGGLE_RTRIM_TOP,
		///
		TOGGLE_LTRIM_BOTTOM,
		///
		TOGGLE_RTRIM_BOTTOM,
		///
		ALIGN_LEFT,
		///
		ALIGN_RIGHT,
		///
		ALIGN_CENTER,
		///
		ALIGN_BLOCK,
		///
		ALIGN_DECIMAL,
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
		SET_MULTICOLUMN,
		///
		UNSET_MULTICOLUMN,
		///
		MULTIROW,
		///
		SET_MULTIROW,
		///
		UNSET_MULTIROW,
		///
		SET_MROFFSET,
		///
		SET_ALL_LINES,
		///
		RESET_FORMAL_DEFAULT,
		///
		UNSET_ALL_LINES,
		///
		TOGGLE_LONGTABULAR,
		///
		SET_LONGTABULAR,
		///
		UNSET_LONGTABULAR,
		///
		SET_PWIDTH,
		///
		SET_MPWIDTH,
		///
		TOGGLE_VARWIDTH_COLUMN,
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
		UNSET_LTNEWPAGE,
		///
		TOGGLE_LTCAPTION,
		///
		SET_LTCAPTION,
		///
		UNSET_LTCAPTION,
		///
		SET_SPECIAL_COLUMN,
		///
		SET_SPECIAL_MULTICOLUMN,
		///
		TOGGLE_BOOKTABS,
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
		TABULAR_VALIGN_TOP,
		///
		TABULAR_VALIGN_MIDDLE,
		///
		TABULAR_VALIGN_BOTTOM,
		///
		LONGTABULAR_ALIGN_LEFT,
		///
		LONGTABULAR_ALIGN_CENTER,
		///
		LONGTABULAR_ALIGN_RIGHT,
		///
		SET_DECIMAL_POINT,
		///
		SET_TABULAR_WIDTH,
		///
		SET_INNER_LINES,
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
		CELL_PART_OF_MULTICOLUMN,
		///
		CELL_BEGIN_OF_MULTIROW,
		///
		CELL_PART_OF_MULTIROW
	};

	///
	enum VAlignment {
		///
		LYX_VALIGN_TOP = 0,
		///
		LYX_VALIGN_MIDDLE = 1,
		///
		LYX_VALIGN_BOTTOM = 2

	};
	///
	enum HAlignment {
		///
		LYX_LONGTABULAR_ALIGN_LEFT = 0,
		///
		LYX_LONGTABULAR_ALIGN_CENTER = 1,
		///
		LYX_LONGTABULAR_ALIGN_RIGHT = 2
	};

	enum BoxType {
		///
		BOX_NONE = 0,
		///
		BOX_PARBOX = 1,
		///
		BOX_MINIPAGE = 2,
		///
		BOX_VARWIDTH = 3
	};

	enum CaptionType {
		///
		CAPTION_FIRSTHEAD,
		///
		CAPTION_HEAD,
		///
		CAPTION_FOOT,
		///
		CAPTION_LASTFOOT,
		///
		CAPTION_ANY
	};

	enum RowDirection {
		UP,
		DOWN
	};

	enum ColDirection {
		RIGHT,
		LEFT
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

	/// index indicating an invalid position
	static const idx_type npos = static_cast<idx_type>(-1);

	/// constructor
	Tabular(Buffer * buf, col_type columns_arg, row_type rows_arg);

	/// Returns true if there is a topline, returns false if not
	bool topLine(idx_type cell) const;
	/// Returns true if there is a topline, returns false if not
	bool bottomLine(idx_type cell) const;
	/// Returns true if there is a topline, returns false if not
	/// If \p ignore_bt is true, we return the state as if booktabs was
	/// not used
	bool leftLine(idx_type cell, bool const ignore_bt = false) const;
	/// Returns true if there is a topline, returns false if not
	/// If \p ignore_bt is true, we return the state as if booktabs was
	/// not used
	bool rightLine(idx_type cell, bool const ignore_bt = false) const;
	/// Returns whether the top line is trimmed left and/or right
	std::pair<bool, bool> topLineTrim(idx_type const cell) const;
	/// Returns whether the bottom line is trimmed left and/or right
	std::pair<bool, bool> bottomLineTrim(idx_type const cell) const;

	/// return space occupied by the second horizontal line and
	/// interline space above row \p row in pixels
	int interRowSpace(row_type row) const;
	///
	int interColumnSpace(idx_type cell) const;

	/* returns the maximum over all rows */
	///
	int cellWidth(idx_type cell) const;
	///
	int cellHeight(idx_type cell) const;
	///
	int width() const;
	///
	int height() const;
	///
	row_type nrows() const {return row_info.size();}
	///
	col_type ncols() const {return column_info.size();}
	///
	int rowAscent(row_type row) const;
	///
	int rowDescent(row_type row) const;
	///
	void setRowAscent(row_type row, int height);
	///
	void setRowDescent(row_type row, int height);
	///
	void setTopLine(idx_type cell, bool line);
	///
	void setBottomLine(idx_type cell, bool line);
	///
	void setTopLineLTrim(idx_type cell, bool val);
	///
	void setBottomLineLTrim(idx_type cell, bool val);
	///
	void setTopLineRTrim(idx_type cell, bool val);
	///
	void setBottomLineRTrim(idx_type cell, bool val);
	///
	void setTopLineTrim(idx_type cell, std::pair<bool, bool>);
	///
	void setBottomLineTrim(idx_type cell, std::pair<bool, bool>);
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
	void setTabularWidth(Length const & l) { tabular_width = l; }
	///
	Length tabularWidth() const { return tabular_width; }
	///
	void setColumnPWidth(Cursor &, idx_type, Length const &);
	///
	bool setMColumnPWidth(Cursor &, idx_type, Length const &);
	///
	bool toggleVarwidth(idx_type, bool const);
	///
	bool setMROffset(Cursor &, idx_type, Length const &);
	///
	void setAlignSpecial(idx_type cell, docstring const & special,
			     Feature what);
	///
	LyXAlignment getAlignment(idx_type cell,
				  bool onlycolumn = false) const;
	///
	VAlignment getVAlignment(idx_type cell,
				 bool onlycolumn = false) const;
	/// The vertical offset of the table due to the vertical
	/// alignment with respect to the baseline.
	int offsetVAlignment() const;
	///
	Length const getPWidth(idx_type cell) const;
	///
	Length const getMROffset(idx_type cell) const;
	///
	int textHOffset(idx_type cell) const;
	///
	int textVOffset(idx_type cell) const;
	///
	void appendRow(row_type row);
	///
	void deleteRow(row_type row, bool const force = false);
	///
	void copyRow(row_type row);
	///
	void insertRow(row_type row, bool copy);
	///
	void moveColumn(col_type col_start, col_type col_end,
			ColDirection direction);
	///
	void moveRow(row_type row_start, row_type row_end,
		     RowDirection direction);
	///
	void appendColumn(col_type column);
	///
	void deleteColumn(col_type column, bool const force = false);
	///
	void copyColumn(col_type column);
	///
	void insertColumn(col_type column, bool copy);
	///
	idx_type getFirstCellInRow(row_type row, bool const ct = false) const;
	///
	idx_type getLastCellInRow(row_type row, bool const ct = false) const;
	///
	idx_type getFirstRow(bool const ct = false) const;
	///
	idx_type getLastRow(bool const ct = false) const;
	///
	idx_type numberOfCellsInRow(row_type row) const;
	///
	void write(std::ostream &) const;
	///
	void read(Lexer &);
	///
	void latex(otexstream &, OutputParams const &) const;
	/// serialise the table in DocBook, according to buffer parameters
	void docbook(XMLStream &, OutputParams const &) const;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const;
	///
	void plaintext(odocstringstream &,
		       OutputParams const & runparams, int const depth,
		       bool onlydata, char_type delim, size_t max_length = INT_MAX) const;
	///
	bool isMultiColumn(idx_type cell) const;
	///
	bool hasMultiColumn(col_type cell) const;
	///
	bool hasVarwidthColumn() const;
	///
	bool isVTypeColumn(col_type cell) const;
	///
	idx_type setMultiColumn(Cursor & cur, idx_type cell, idx_type number,
			     bool const right_border);
	///
	void unsetMultiColumn(idx_type cell);
	///
	bool isPartOfMultiColumn(row_type row, col_type column) const;
	///
	bool isPartOfMultiRow(row_type row, col_type column) const;
	///
	bool isMultiRow(idx_type cell) const;
	///
	bool hasMultiRow(row_type r) const;
	///
	idx_type setMultiRow(Cursor & cur, idx_type cell, idx_type number,
			     bool const bottom_border,
			     LyXAlignment const halign);
	///
	void unsetMultiRow(idx_type cell);
	///
	row_type cellRow(idx_type cell) const;
	///
	col_type cellColumn(idx_type cell) const;
	///
	void setRotateCell(idx_type cell, int);
	///
	int getRotateCell(idx_type cell) const;
	///
	bool needRotating() const;
	///
	bool isLastCell(idx_type cell) const;
	///
	idx_type cellAbove(idx_type cell) const;
	///
	idx_type cellBelow(idx_type cell) const;
	/// \return the index of the VISIBLE cell at row, column
	/// this will be the same as the cell in the previous row,
	/// e.g., if the cell is part of a multirow
	idx_type cellIndex(row_type row, col_type column) const;
	///
	void setUsebox(idx_type cell, BoxType);
	///
	BoxType getUsebox(idx_type cell) const;
	//
	// Long Tabular Options support functions
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
	idx_type setLTCaption(Cursor & cur, row_type row, bool what);
	///
	bool ltCaption(row_type row) const;
	///
	bool haveLTHead(bool withcaptions = true) const;
	///
	bool haveLTFirstHead(bool withcaptions = true) const;
	///
	bool haveLTFoot(bool withcaptions = true) const;
	///
	bool haveLTLastFoot(bool withcaptions = true) const;
	///
	bool haveLTCaption(CaptionType captiontype = CAPTION_ANY) const;
	///
	// end longtable support

	//@{
	/// there is a subtle difference between these two methods.
	///   cellInset(r,c);
	/// and
	///   cellInset(cellIndex(r,c));
	/// can return different things. this is because cellIndex(r,c)
	/// returns the VISIBLE cell at r,c, which may be the same as the
	/// cell at the previous row or column, if we're dealing with some
	/// multirow or multicell.
	std::shared_ptr<InsetTableCell> cellInset(idx_type cell);
	std::shared_ptr<InsetTableCell> cellInset(row_type row, col_type column);
	InsetTableCell const * cellInset(idx_type cell) const;
	//@}
	///
	void setCellInset(row_type row, col_type column,
	                  std::shared_ptr<InsetTableCell>);
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
		explicit CellData(Buffer *);
		///
		CellData(CellData const &);
		///
		CellData & operator=(CellData const &);
		///
		idx_type cellno;
		///
		int width;
		///
		int multicolumn;
		///
		int multirow;
		///
		Length mroffset;
		///
		LyXAlignment alignment;
		///
		VAlignment valignment;
		/// width of the part before the decimal
		int decimal_hoffset;
		/// width of the decimal part
		int decimal_width;
		///
		int voffset;
		///
		bool top_line;
		///
		bool bottom_line;
		///
		bool left_line;
		///
		bool right_line;
		///
		bool top_line_rtrimmed;
		///
		bool top_line_ltrimmed;
		///
		bool bottom_line_rtrimmed;
		///
		bool bottom_line_ltrimmed;
		///
		BoxType usebox;
		///
		int rotate;
		///
		docstring align_special;
		///
		Length p_width; // this is only set for multicolumn!!!
		///
		std::shared_ptr<InsetTableCell> inset;
	};
	///
	CellData const & cellInfo(idx_type cell) const;
	///
	CellData & cellInfo(idx_type cell);
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
		///
		Change change;
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
		///
		docstring decimal_point;
		///
		bool varwidth;
		///
		Change change;
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
	cell_vvector cell_info;
	///
	Length tabular_width;
	///
	bool use_booktabs;
	///
	int rotate;
	///
	VAlignment tabular_valignment;
	//
	// for long tabulars
	///
	HAlignment longtabular_alignment;
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
	void init(Buffer *, row_type rows_arg,
		  col_type columns_arg);
	///
	void updateIndexes();
	///
	bool setFixedWidth(row_type r, col_type c);
	/// return true of update is needed
	bool updateColumnWidths(MetricsInfo & mi);
	///
	idx_type columnSpan(idx_type cell) const;
	///
	idx_type rowSpan(idx_type cell) const;
	///
	BoxType useBox(idx_type cell) const;
	///
	// helper function for Latex
	///
	void TeXTopHLine(otexstream &, row_type row, std::list<col_type>,
			 std::list<col_type>) const;
	///
	void TeXBottomHLine(otexstream &, row_type row, std::list<col_type>,
			    std::list<col_type>) const;
	///
	void TeXCellPreamble(otexstream &, idx_type cell, bool & ismulticol, bool & ismultirow,
			     bool const bidi) const;
	///
	void TeXCellPostamble(otexstream &, idx_type cell, bool ismulticol, bool ismultirow) const;
	///
	void TeXLongtableHeaderFooter(otexstream &, OutputParams const &, std::list<col_type>,
				      std::list<col_type>) const;
	///
	bool isValidRow(row_type const row) const;
	///
	void TeXRow(otexstream &, row_type const row,
		    OutputParams const &, std::list<col_type>, std::list<col_type>) const;
	///
	// helper functions for plain text
	///
	bool plaintextTopHLine(odocstringstream &, row_type row,
			       std::vector<unsigned int> const &) const;
	///
	bool plaintextBottomHLine(odocstringstream &, row_type row,
				  std::vector<unsigned int> const &) const;
	///
	void plaintextPrintCell(odocstringstream &,
				OutputParams const &,
				idx_type cell, row_type row, col_type column,
				std::vector<unsigned int> const &,
				bool onlydata, size_t max_length) const;
	/// auxiliary function for DocBook
	void docbookRow(XMLStream &, row_type, OutputParams const &,
					bool header = false) const;
	/// auxiliary function for DocBook: export this row as HTML
	void docbookRowAsHTML(XMLStream &, row_type, OutputParams const &,
					bool header) const;
	/// auxiliary function for DocBook: export this row as CALS
	void docbookRowAsCALS(XMLStream &, row_type, OutputParams const &) const;
	///
	docstring xhtmlRow(XMLStream & xs, row_type, OutputParams const &,
	                   bool header = false) const;

	/// change associated Buffer
	void setBuffer(Buffer & buffer);
	/// retrieve associated Buffer
	Buffer const & buffer() const { return *buffer_; }
	/// retrieve associated Buffer
	Buffer & buffer() { return *buffer_; }

private:
	Buffer * buffer_;

}; // Tabular


class InsetTabular : public Inset
{
public:
	///
	InsetTabular(Buffer *, row_type rows = 1,
		     col_type columns = 1);
	///
	~InsetTabular();
	///
	void setBuffer(Buffer & buffer) override;

	///
	static void string2params(std::string const &, InsetTabular &);
	///
	static std::string params2string(InsetTabular const &);
	///
	void read(Lexer &) override;
	///
	void write(std::ostream &) const override;
	///
	void metrics(MetricsInfo &, Dimension &) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void drawSelection(PainterInfo & pi, int x, int y) const override;
	///
	void drawBackground(PainterInfo & pi, int x, int y) const override;
	///
	bool editable() const override { return true; }
	///
	bool hasSettings() const override { return true; }
	///
	bool insetAllowed(InsetCode code) const override;
	///
	bool allowSpellCheck() const override { return true; }
	///
	bool canTrackChanges() const override { return true; }
	///
	bool canPaintChange(BufferView const &) const override { return true; }
	/** returns false if, when outputting LaTeX, font changes should
	    be closed before generating this inset. This is needed for
	    insets that may contain several paragraphs */
	bool inheritFont() const override { return false; }
	///
	bool allowMultiPar() const override;
	///
	bool allowsCaptionVariation(std::string const &) const override;
	//
	bool isTable() const override { return true; }
	///
	RowFlags rowFlags() const override;
	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	InsetCode lyxCode() const override { return TABULAR_CODE; }
	///
	std::string contextMenu(BufferView const &, int, int) const override;
	///
	std::string contextMenuName() const override;
	/// get offset of this cursor slice relative to our upper left corner
	void cursorPos(BufferView const & bv, CursorSlice const & sl,
		bool boundary, int & x, int & y) const override;
	/// Executes a space-separated sequence of tabular-features requests
	void tabularFeatures(Cursor & cur, std::string const & what);
	/// Change a single tabular feature; does not handle undo.
	void tabularFeatures(Cursor & cur, Tabular::Feature feature,
			     std::string const & val = std::string());
	/// number of cells
	size_t nargs() const override { return tabular.numberofcells; }
	///
	std::shared_ptr<InsetTableCell const> cell(idx_type) const;
	///
	std::shared_ptr<InsetTableCell> cell(idx_type);
	///
	Text * getText(int) const override;

	/// does the inset contain changes ?
	bool isChanged() const override;
	/// set the change for the entire inset
	void setChange(Change const & change) override;
	/// accept the changes within the inset
	void acceptChanges() override;
	/// reject the changes within the inset
	void rejectChanges() override;

	// this should return true if we have a "normal" cell, otherwise false.
	// "normal" means without width set!
	/// should all paragraphs be output with "Standard" layout?
	bool allowParagraphCustomization(idx_type cell = 0) const override;
	///
	void addPreview(DocIterator const & inset_pos,
		graphics::PreviewLoader &) const override;

	/// lock cell with given index
	void edit(Cursor & cur, bool front, EntryDirection entry_from) override;
	/// get table row from x coordinate
	int rowFromY(Cursor & cur, int y) const;
	/// get table column from y coordinate
	int columnFromX(Cursor & cur, int x) const;
	///
	Inset * editXY(Cursor & cur, int x, int y) override;
	/// can we go further down on mouse click?
	bool descendable(BufferView const &) const override { return true; }
	/// Update the counters of this inset and of its contents
	void updateBuffer(ParIterator const &, UpdateType, bool const deleted = false) override;
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype, TocBackend & backend) const override;

	///
	bool completionSupported(Cursor const &) const override;
	///
	bool inlineCompletionSupported(Cursor const & cur) const override;
	///
	bool automaticInlineCompletion() const override;
	///
	bool automaticPopupCompletion() const override;
	///
	bool showCompletionCursor() const override;
	///
	CompletionList const * createCompletionList(Cursor const & cur) const override;
	///
	docstring completionPrefix(Cursor const & cur) const override;
	///
	bool insertCompletion(Cursor & cur, docstring const & s, bool finished) override;
	///
	void completionPosAndDim(Cursor const &, int & x, int & y, Dimension & dim) const override;
	///
	bool usePlainLayout() const override { return true; }
	///
	docstring layoutName() const override { return from_ascii("Tabular"); }


	///
	InsetTabular * asInsetTabular() override { return this; }
	///
	InsetTabular const * asInsetTabular() const override { return this; }
	///
	bool isRightToLeft(Cursor & cur) const;
	/// writes the cells between stidx and enidx as a string, optionally
	/// descending into the insets
	docstring asString(idx_type stidx, idx_type enidx, bool intoInsets = true);
	///
	ParagraphList asParList(idx_type stidx, idx_type enidx);

	/// Returns whether the cell in the specified row and column is selected.
	bool isCellSelected(Cursor & cur, row_type row, col_type col) const;
	///
	void setLayoutForHiddenCells(DocumentClass const & dc);
	//
	// Public structures and variables
	///
	mutable Tabular tabular;

private:
	///
	InsetTabular(InsetTabular const &);
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	bool getFeatureStatus(Cursor & cur, std::string const & s,
	                 std::string const & argument, FuncStatus & status) const;
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const override;
	///
	Inset * clone() const override { return new InsetTabular(*this); }

	///
	bool hitSelectRow(BufferView const & bv, int x) const;
	///
	bool hitSelectColumn(BufferView const & bv, int y) const;
	/// Returns true if coordinates are on row/column selection zones
	bool clickable(BufferView const &, int x, int y) const override;

	///
	void drawCellLines(PainterInfo &, int x, int y, row_type row,
			   idx_type cell) const;
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
	int cellYPos(idx_type cell) const;
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

	/// return the "Manhattan distance" to nearest corner
	int dist(BufferView &, idx_type cell, int x, int y) const;
	/// return the cell nearest to x, y
	idx_type getNearestCell(BufferView &, int x, int y) const;

	/// test the rotation state of the given cell range.
	bool oneCellHasRotationState(bool rotated,
				row_type row_start, row_type row_end,
				col_type col_start, col_type col_end) const;

	/// true when selecting rows with the mouse
	bool rowselect_;
	/// true when selecting columns with the mouse
	bool colselect_;
};

std::string const featureAsString(Tabular::Feature feature);

/// Split cell on decimal symbol
InsetTableCell splitCell(InsetTableCell & head, docstring const & decimal_sym, bool & hassep);

} // namespace lyx

#endif // INSET_TABULAR_H
