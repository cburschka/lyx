// -*- C++ -*-
#ifndef MATH_GRID_H
#define MATH_GRID_H

#include "math_nestinset.h"
#include "vspace.h"
#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Gridded math inset base class.
    This is the base to all grid-like editable math objects
    like array and eqnarray.
    \author André Pönitz 2001
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
		string align_;
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
	MathGridInset(char valign, string const & halign);
	/// Note: columns first!
	MathGridInset(col_type m, row_type n);
	///
	MathGridInset(col_type m, row_type n, char valign, string const & halign);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi) const;
	///
	void drawT(TextPainter & pi, int x, int y) const;
	///
	void halign(string const & align);
	///
	void halign(char c, col_type col);
	///
	char halign(col_type col) const;
	///
	string halign() const;
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
	virtual MathGridInset * asGridInset() { return this; }

	///
	col_type ncols() const;
	///
	row_type nrows() const;
	///
	col_type col(idx_type idx) const;
	///
	row_type row(idx_type idx) const;

	///
	bool idxUpDown(idx_type & idx, pos_type & pos, bool up, int targetx) const;
	///
	bool idxLeft(idx_type & idx, pos_type & pos) const;
	///
	bool idxRight(idx_type & idx, pos_type & pos) const;
	///
	bool idxFirst(idx_type & idx, pos_type & pos) const;
	///
	bool idxLast(idx_type & idx, pos_type & pos) const;
	///
	bool idxHome(idx_type & idx, pos_type & pos) const;
	///
	bool idxEnd(idx_type & idx, pos_type & pos) const;
	///
	bool idxDelete(idx_type & idx);
	/// pulls cell after pressing erase
	void idxGlue(idx_type idx);

	///
	virtual void addRow(row_type r);
	///
	virtual void delRow(row_type r);
	///
	virtual void addFancyRow(row_type r) { addRow(r); }
	///
	virtual void delFancyRow(row_type r) { delRow(r); }
	///
	virtual void addCol(col_type c);
	///
	virtual void delCol(col_type c);
	///
	virtual void addFancyCol(col_type c) { addCol(c); }
	///
	virtual void delFancyCol(col_type c) { delCol(c); }
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
	//void maplize(MapleStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	//void octavize(OctaveStream &) const;

protected:
	/// returns x offset of cell compared to inset
	int cellXOffset(idx_type idx) const;
	/// returns y offset of cell compared to inset
	int cellYOffset(idx_type idx) const;
	/// returns proper 'end of line' code for LaTeX
	string eolString(row_type row, bool fragile = false) const;
	/// returns proper 'end of column' code for LaTeX
	string eocString(col_type col) const;
	/// extract number of columns from alignment string
	col_type guessColumns(string const & halign) const;

public:
	/// row info
	std::vector<RowInfo> rowinfo_;
	/// column info
	std::vector<ColInfo> colinfo_;
	/// cell info
	std::vector<CellInfo> cellinfo_;
	///
	char v_align_; // add approp. type
};

#endif
