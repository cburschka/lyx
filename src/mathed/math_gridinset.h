// -*- C++ -*-
/**
 * \file math_gridinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_GRID_H
#define MATH_GRID_H

#include "math_nestinset.h"
#include "lyxlength.h"


/** Gridded math inset base class.
 *  This is the base to all grid-like editable math objects
 */
class MathGridInset : public MathNestInset {
public:

	/// additional per-cell information
	struct CellInfo {
		///
		CellInfo();
		/// a dummy cell before a multicolumn cell
		int dummy_;
		/// special multi colums alignment
		std::string align_;
		/// these should be a per-cell property, but ok to have it here
		/// for single-column grids like paragraphs
		mutable int glue_;
		///
		mutable pos_type begin_;
		///
		mutable pos_type end_;
	};

	/// additional per-row information
	struct RowInfo {
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
		int lines_;
		/// parameter to the line break
		LyXLength crskip_;
		/// extra distance between lines
		int skip_;
	};

	// additional per-row information
	struct ColInfo {
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
		/// do we need a line to the left?
		bool leftline_;
		/// do we need a line to the right?
		bool rightline_;
		/// how many lines to the left of this column?
		int lines_;
		/// additional amount to be skipped when drawing
		int skip_;
	};

public:
	/// sets nrows and ncols to 1
	MathGridInset();
	/// constructor from columns description, creates one row
	MathGridInset(char valign, std::string const & halign);
	/// Note: columns first!
	MathGridInset(col_type m, row_type n);
	///
	MathGridInset(col_type m, row_type n, char valign, std::string const & halign);
	/// Ensures that the dialog is closed.
	~MathGridInset();
	///
	void metrics(MetricsInfo & mi) const;
	///
	void metrics(MetricsInfo & mi, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const;
	///
	void drawT(TextPainter & pi, int x, int y) const;
	///
	void halign(std::string const & align);
	///
	void halign(char c, col_type col);
	///
	char halign(col_type col) const;
	///
	std::string halign() const;
	///
	void valign(char c);
	///
	char valign() const;
	///
	void vcrskip(LyXLength const &, row_type row);
	///
	LyXLength vcrskip(row_type row) const;
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
	MathGridInset * asGridInset() { return this; }
	/// identifies GridInset
	MathGridInset const * asGridInset() const { return this; }
	///
	col_type ncols() const;
	///
	row_type nrows() const;
	///
	col_type col(idx_type idx) const;
	///
	row_type row(idx_type idx) const;

	///
	bool idxUpDown(LCursor &, bool up) const;
	///
	bool idxLeft(LCursor &) const;
	///
	bool idxRight(LCursor &) const;
	///
	bool idxFirst(LCursor &) const;
	///
	bool idxLast(LCursor &) const;
	///
	bool idxDelete(idx_type & idx);
	/// pulls cell after pressing erase
	void idxGlue(idx_type idx);

	///
	virtual void addRow(row_type r);
	///
	virtual void delRow(row_type r);
	///
	virtual void copyRow(row_type r);
	///
	virtual void swapRow(row_type r);
	///
	virtual void addCol(col_type c);
	///
	virtual void delCol(col_type c);
	///
	virtual void copyCol(col_type c);
	///
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
	void mathmlize(MathMLStream &) const;
	///
	//void octave(OctaveStream &) const;

protected:
	///
	void priv_dispatch(LCursor & cur, FuncRequest & cmd);
	///
	bool getStatus(LCursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const;
	/// returns x offset of cell compared to inset
	int cellXOffset(idx_type idx) const;
	/// returns y offset of cell compared to inset
	int cellYOffset(idx_type idx) const;
	/// returns proper 'end of line' code for LaTeX
	virtual std::string eolString(row_type row, bool fragile = false) const;
	/// returns proper 'end of column' code for LaTeX
	virtual std::string eocString(col_type col, col_type lastcol) const;
	/// extract number of columns from alignment string
	col_type guessColumns(std::string const & halign) const;
	/// splits cells and shifts right part to the next cell
	void splitCell(LCursor & cur);

public:
	/// row info
	std::vector<RowInfo> rowinfo_;
	/// column info
	std::vector<ColInfo> colinfo_;
	/// cell info
	std::vector<CellInfo> cellinfo_;
	///
	char v_align_; // add approp. type
private:
	virtual std::auto_ptr<InsetBase> doClone() const;
};

#endif
