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
	MathGridInset(unsigned int m, unsigned int n);
	///
	void write(std::ostream &, bool fragile) const;
	///
	void metrics(MathStyles st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void halign(string const &);
	///
	void halign(char c, unsigned int col);
	///
	char halign(unsigned int col) const;
	///
	void valign(char c);
	///
	char valign() const;
	///
	void vskip(LyXLength const &, unsigned int row);
	///
	LyXLength vskip(unsigned int row) const;
	///
	void resize(short int type, unsigned int cols);
	///
	const RowInfo & rowinfo(unsigned int row) const;
	///
	RowInfo & rowinfo(unsigned int row);
	///
	bool isGrid() const { return true; }

	///
	unsigned int ncols() const { return colinfo_.size(); }
	///
	unsigned int nrows() const { return rowinfo_.size(); }
	///
	unsigned int col(unsigned int idx) const { return idx % ncols(); }
	///
	unsigned int row(unsigned int idx) const { return idx / ncols(); }
	///
	int cellXOffset(unsigned int idx) const;
	///
	int cellYOffset(unsigned int idx) const;

	///
	bool idxUp(unsigned int &, unsigned int &) const;
	///
	bool idxDown(unsigned int &, unsigned int &) const;
	///
	bool idxLeft(unsigned int &, unsigned int &) const;
	///
	bool idxRight(unsigned int &, unsigned int &) const;
	///
	bool idxFirst(unsigned int &, unsigned int &) const;
	///
	bool idxLast(unsigned int &, unsigned int &) const;
	///
	void idxDelete(unsigned int &, bool &, bool &);
	///
	void idxDeleteRange(unsigned int, unsigned int);
			
	///
	void addRow(unsigned int);
	///
	void delRow(unsigned int);
	///
	void addCol(unsigned int);
	///
	void delCol(unsigned int);
	///
	virtual void appendRow();
	///
	unsigned int index(unsigned int row, unsigned int col) const;
	///
	std::vector<unsigned int> idxBetween(unsigned int from, unsigned int to) const;
	///
	virtual int defaultColSpace(unsigned int) { return 10; }
	///
	virtual char defaultColAlign(unsigned int) { return 'c'; }
	///
	void setDefaults();

protected:
	/// returns proper 'end of line' code for LaTeX
	string eolString(unsigned int row) const;
	/// returns proper 'end of column' code for LaTeX
	string eocString(unsigned int col) const;

	/// row info
	std::vector<RowInfo> rowinfo_;
	/// column info
	std::vector<ColInfo> colinfo_;
	/// 
	char v_align_; // add approp. type
};

#endif
