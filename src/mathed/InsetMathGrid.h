// -*- C++ -*-
/**
 * \file InsetMathGrid.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_GRID_H
#define MATH_GRID_H

#include "InsetMathNest.h"
#include "Length.h"


namespace lyx {


/** Gridded math inset base class.
 *  This is the base to all grid-like editable math objects
 */
class InsetMathGrid : public InsetMathNest {
public:

	/// additional per-cell information
	class CellInfo {
	public:
		///
		CellInfo();
		/// a dummy cell before a multicolumn cell
		int dummy_;
		/// special multi colums alignment
		docstring align_;
		/// these should be a per-cell property, but ok to have it here
		/// for single-column grids like paragraphs
		mutable int glue_;
		///
		mutable pos_type begin_;
		///
		mutable pos_type end_;
	};

	/// additional per-row information
	class RowInfo {
	public:
		///
		RowInfo();
		///
		int skipPixels() const;
		/// cached descent
		mutable int descent_;
		/// cached ascent
		mutable int ascent_;
		/// cached offset
		mutable int offset_;
		/// how many hlines above this row?
		unsigned int lines_;
		/// parameter to the line break
		Length crskip_;
		/// extra distance between lines
		int skip_;
		/// Is a page break allowed after this row?
		bool allow_pagebreak_;
	};

	// additional per-row information
	class ColInfo {
	public:
		///
		ColInfo();
		/// currently possible: 'l', 'c', 'r'
		char align_;
		/// cache for drawing
		int h_offset;
		/// cached width
		mutable int width_;
		/// cached offset
		mutable int offset_;
		/// how many lines to the left of this column?
		unsigned int lines_;
		/// additional amount to be skipped when drawing
		int skip_;
		/// Special alignment.
		/// This does also contain align_ and lines_ if it is nonempty.
		/// It needs to be in sync with align_ and lines_ because some
		/// code only uses align_ and lines_.
		docstring special_;
	};

public:
	/// sets nrows and ncols to 1
	InsetMathGrid();
	/// constructor from columns description, creates one row
	InsetMathGrid(char valign, docstring const & halign);
	/// Note: columns first!
	InsetMathGrid(col_type m, row_type n);
	///
	InsetMathGrid(col_type m, row_type n, char valign, docstring const & halign);
	///
	void metrics(MetricsInfo & mi) const;
	///
	bool metrics(MetricsInfo & mi, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void drawWithMargin(PainterInfo & pi, int x, int y,
		int lmargin = 0, int rmargin = 0) const;
	/// draw decorations.
	void drawDecoration(PainterInfo & pi, int x, int y) const
	{ drawMarkers2(pi, x, y); }
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const;
	///
	void drawT(TextPainter & pi, int x, int y) const;
	///
	void halign(docstring const & align);
	///
	void halign(char c, col_type col);
	///
	char halign(col_type col) const;
	///
	docstring halign() const;
	///
	void valign(char c);
	///
	char valign() const;
	///
	void vcrskip(Length const &, row_type row);
	///
	Length vcrskip(row_type row) const;
	///
	void resize(short int type, col_type cols);
	///
	const RowInfo & rowinfo(row_type row) const;
	/// returns topmost row if passed (-1)
	RowInfo & rowinfo(row_type row);
	///
	const CellInfo & cellinfo(idx_type idx) const { return cellinfo_[idx]; }
	///
	CellInfo & cellinfo(idx_type idx) { return cellinfo_[idx]; }
	/// identifies GridInset
	InsetMathGrid * asGridInset() { return this; }
	/// identifies GridInset
	InsetMathGrid const * asGridInset() const { return this; }
	///
	col_type ncols() const;
	///
	row_type nrows() const;
	///
	col_type col(idx_type idx) const;
	///
	row_type row(idx_type idx) const;

	///
	bool idxUpDown(Cursor &, bool up) const;
	///
	bool idxLeft(Cursor &) const;
	///
	bool idxRight(Cursor &) const;
	///
	bool idxFirst(Cursor &) const;
	///
	bool idxLast(Cursor &) const;
	///
	bool idxDelete(idx_type & idx);
	/// pulls cell after pressing erase
	void idxGlue(idx_type idx);

	/// add a row
	virtual void addRow(row_type r);
	/// delete a row
	virtual void delRow(row_type r);
	/// copy a row
	virtual void copyRow(row_type r);
	/// swap two rows
	virtual void swapRow(row_type r);
	/// add a column
	virtual void addCol(col_type c);
	/// delete a column
	virtual void delCol(col_type c);
	/// copy a column
	virtual void copyCol(col_type c);
	/// swap two columns
	virtual void swapCol(col_type c);
	///
	virtual void appendRow();
	///
	idx_type index(row_type r, col_type c) const;
	///
	bool idxBetween(idx_type idx, idx_type from, idx_type to) const;
	///
	virtual int defaultColSpace(col_type) { return 0; }
	///
	virtual char defaultColAlign(col_type) { return 'c'; }
	///
	void setDefaults();

	///
	virtual int colsep() const;
	///
	virtual int rowsep() const;
	///
	virtual int hlinesep() const;
	///
	virtual int vlinesep() const;
	///
	virtual int border() const;

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	//void maple(MapleStream &) const;
	///
	void mathmlize(MathStream &) const;
	///
	//void octave(OctaveStream &) const;

protected:
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const;
	/// returns x offset of cell compared to inset
	int cellXOffset(idx_type idx) const;
	/// returns y offset of cell compared to inset
	int cellYOffset(idx_type idx) const;
	/// returns proper 'end of line' code for LaTeX
	virtual docstring eolString(row_type row, bool emptyline,
				      bool fragile) const;
	/// returns proper 'end of column' code for LaTeX
	virtual docstring eocString(col_type col, col_type lastcol) const;
	/// extract number of columns from alignment string
	col_type guessColumns(docstring const & halign) const;
	/// splits cells and shifts right part to the next cell
	void splitCell(Cursor & cur);

	/// row info.
	/// rowinfo_[nrows()] is a dummy row used only for hlines.
	std::vector<RowInfo> rowinfo_;
	/// column info.
	/// colinfo_[ncols()] is a dummy column used only for vlines.
	std::vector<ColInfo> colinfo_;
	/// cell info
	std::vector<CellInfo> cellinfo_;
	///
	char v_align_; // add approp. type
private:
	virtual std::auto_ptr<Inset> doClone() const;
};



} // namespace lyx
#endif
