// -*- C++ -*-
#ifndef MATH_GRID_H
#define MATH_GRID_H

#include "math_nestinset.h"

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
		int descent_;
		///
		int ascent_;
		/// 
		int offset_;
		///
		bool upperline_;
		///
		bool lowerline_;
	};

	// additional per-row information
	struct ColInfo {
		///	
		ColInfo();
		///
		char h_align_;
		/// cache for drawing
		int h_offset;
		///
		int width_;
		///
		int offset_;
		///
		bool leftline_;
		///
		bool rightline_;
	};

public: 
	///
	MathGridInset(int m, int n, string const & nm);
	///
	virtual MathInset * clone() const = 0;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void metrics(MathStyles st);
	///
	void draw(Painter &, int, int);
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

protected:
	/// row info
	std::vector<RowInfo> rowinfo_;
	/// column info
	std::vector<ColInfo> colinfo_;
	/// 
	char v_align_; // add approp. type
};

#endif
