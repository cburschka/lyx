// -*- C++ -*-
#ifndef MATH_MATRIXINSET_H
#define MATH_MATRIXINSET_H

#include "math_gridinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Multiline math paragraph base class.
    This is the base to all multiline editable math objects
    like array and eqnarray.
    \author Alejandro Aguilar Sierra
*/

class LaTeXFeatures;

class MathMatrixInset : public MathGridInset {
public: 
	///
	MathMatrixInset();
	///
	explicit MathMatrixInset(MathInsetTypes t);
	///
	MathMatrixInset(MathInsetTypes t, col_type cols);
	///
	MathInset * clone() const;
	///
	void write(MathWriteInfo & os) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	string label(row_type row) const;
	///
	void label(row_type row, string const & label);
	///
	void numbered(row_type row, bool num);
	///
	bool numbered(row_type row) const;
	///
	bool numberedType() const;
	///
	bool display() const;
	///
	bool ams() const;
	///
	std::vector<string> const getLabelList() const;
	///
	void validate(LaTeXFeatures & features) const;
	/// identifies MatrixInsets
	virtual MathMatrixInset const * asMatrixInset() const { return this; }
	/// identifies MatrixInsets
	virtual MathMatrixInset * asMatrixInset() { return this; }

	///
	void addRow(row_type);
	///
	void delRow(row_type);
	///
	void addCol(col_type);
	///
	void delCol(col_type);
	///
	void appendRow();

	/// change type
	void mutate(string const &);
	///
	void mutate(MathInsetTypes);

	///
	int defaultColSpace(col_type col);
	///
	char defaultColAlign(col_type col);

	///
	MathInsetTypes getType() const;

private:
	///
	void setType(MathInsetTypes t);
	///
	void validate1(LaTeXFeatures & features);
	///
	void header_write(std::ostream &) const;
	///
	void footer_write(std::ostream &) const;
	///
	void glueall();
	///
	string nicelabel(row_type row) const;

	///
	MathInsetTypes objtype_;
	///
	std::vector<int> nonum_;
	///
	std::vector<string> label_;
	///
	mutable MathMetricsInfo mi_;
};

#endif
