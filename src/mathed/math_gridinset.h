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
		/// hline abow this row?
		bool upperline_;
		/// hline below this row?
		bool lowerline_;
		/// distance
		LyXLength skip_;
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
		/// additional amount to be skipped when drawing
		int skip_;
	};

public: 
	/// Note: columns first!
	MathGridInset(col_type m, row_type n);
	///
	MathGridInset(int m, int n, char valign, string const & halign);
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathStyles st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void halign(string const &);
	///
	void halign(char c, col_type col);
	///
	char halign(col_type col) const;
	///
	void valign(char c);
	///
	char valign() const;
	///
	void vskip(LyXLength const &, row_type row);
	///
	LyXLength vskip(row_type row) const;
	///
	void resize(short int type, col_type cols);
	///
	const RowInfo & rowinfo(row_type row) const;
	///
	RowInfo & rowinfo(row_type row);
	/// identifies GridInset
	virtual MathGridInset * asGridInset() { return this; }

	///
	col_type ncols() const { return colinfo_.size(); }
	///
	row_type nrows() const { return rowinfo_.size(); }
	///
	col_type col(idx_type idx) const { return idx % ncols(); }
	///
	row_type row(idx_type idx) const { return idx / ncols(); }
	///
	int cellXOffset(idx_type idx) const;
	///
	int cellYOffset(idx_type idx) const;

	///
	bool idxUp(idx_type &, pos_type &) const;
	///
	bool idxDown(idx_type &, pos_type &) const;
	///
	bool idxLeft(idx_type &, pos_type &) const;
	///
	bool idxRight(idx_type &, pos_type &) const;
	///
	bool idxFirst(idx_type &, pos_type &) const;
	///
	bool idxLast(idx_type &, pos_type &) const;
	///
	void idxDelete(idx_type &, bool &, bool &);
	///
	void idxDeleteRange(idx_type, idx_type);
			
	///
	void addRow(row_type);
	///
	void delRow(row_type);
	///
	void addCol(col_type);
	///
	void delCol(col_type);
	///
	virtual void appendRow();
	///
	idx_type index(row_type row, col_type col) const;
	///
	std::vector<idx_type> idxBetween(idx_type from, idx_type to) const;
	///
	virtual int defaultColSpace(col_type) { return 10; }
	///
	virtual char defaultColAlign(col_type) { return 'c'; }
	///
	void setDefaults();

protected:
	/// returns proper 'end of line' code for LaTeX
	string eolString(row_type row) const;
	/// returns proper 'end of column' code for LaTeX
	string eocString(col_type col) const;

	/// row info
	std::vector<RowInfo> rowinfo_;
	/// column info
	std::vector<ColInfo> colinfo_;
	/// 
	char v_align_; // add approp. type
};

#endif
