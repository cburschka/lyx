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
	/// constructor from columns description, creates one row
	MathGridInset(char valign, string const & halign);
	/// Note: columns first!
	MathGridInset(col_type m, row_type n);
	///
	MathGridInset(col_type m, row_type n, char valign, string const & halign);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & st) const;
	///
	void drawT(TextPainter &, int x, int y) const;
	///
	void halign(string const &);
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
	int cellXOffset(idx_type idx) const;
	///
	int cellYOffset(idx_type idx) const;

	///
	bool idxUpDown(idx_type &, bool) const;
	///
	bool idxLeft(idx_type &, pos_type &) const;
	///
	bool idxRight(idx_type &, pos_type &) const;
	///
	bool idxFirst(idx_type &, pos_type &) const;
	///
	bool idxLast(idx_type &, pos_type &) const;
	///
	bool idxHome(idx_type &, pos_type &) const;
	///
	bool idxEnd(idx_type &, pos_type &) const;
	///
	bool idxDelete(idx_type &);
	/// pulls cell after pressing erase
	void idxGlue(idx_type idx);

	///
	virtual void addRow(row_type);
	///
	virtual void delRow(row_type);
	///
	virtual void addCol(col_type);
	///
	virtual void delCol(col_type);
	///
	virtual void appendRow();
	///
	idx_type index(row_type row, col_type col) const;
	///
	std::vector<idx_type> idxBetween(idx_type from, idx_type to) const;
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
	/// returns proper 'end of line' code for LaTeX
	string eolString(row_type row, bool fragile = false) const;
	/// returns proper 'end of column' code for LaTeX
	string eocString(col_type col) const;
	/// extract number of columns from alignment string
	col_type guessColumns(string const & halign) const;

	/// row info
	std::vector<RowInfo> rowinfo_;
	/// column info
	std::vector<ColInfo> colinfo_;
	///
	char v_align_; // add approp. type
};

#endif
