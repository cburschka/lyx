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
	///
	MathGridInset(int m, int n);
	///
	void write(std::ostream &, bool fragile) const;
	///
	void metrics(MathStyles st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void halign(string const &);
	///
	void halign(char c, int col);
	///
	char halign(int col) const;
	///
	void valign(char c);
	///
	char valign() const;
	///
	void vskip(LyXLength const &, int row);
	///
	LyXLength vskip(int row) const;
	///
	void resize(short int type, int cols);
	///
	const RowInfo & rowinfo(int row) const;
	///
	RowInfo & rowinfo(int row);
	///
	bool isGrid() const { return true; }

	///
	int ncols() const { return colinfo_.size(); }
	///
	int nrows() const { return rowinfo_.size(); }
	///
	int col(int idx) const { return idx % ncols(); }
	///
	int row(int idx) const { return idx / ncols(); }
	///
	int cellXOffset(int idx) const;
	///
	int cellYOffset(int idx) const;

	///
	bool idxUp(int &, int &) const;
	///
	bool idxDown(int &, int &) const;
	///
	bool idxLeft(int &, int &) const;
	///
	bool idxRight(int &, int &) const;
	///
	bool idxFirst(int &, int &) const;
	///
	bool idxLast(int &, int &) const;
	///
	void idxDelete(int &, bool &, bool &);
	///
	void idxDeleteRange(int, int);
			
	///
	void addRow(int);
	///
	void delRow(int);
	///
	void addCol(int);
	///
	void delCol(int);
	///
	virtual void appendRow();
	///
	int index(int row, int col) const;
	///
	std::vector<int> idxBetween(int from, int to) const;
	///
	virtual int defaultColSpace(int) { return 10; }
	///
	virtual char defaultColAlign(int) { return 'c'; }
	///
	void setDefaults();

protected:
	/// returns proper 'end of line' code for LaTeX
	string eolString(int row) const;
	/// returns proper 'end of column' code for LaTeX
	string eocString(int col) const;

	/// row info
	std::vector<RowInfo> rowinfo_;
	/// column info
	std::vector<ColInfo> colinfo_;
	/// 
	char v_align_; // add approp. type
};

#endif
