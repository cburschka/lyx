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
	MathInset * clone() const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void metrics(MathStyles st);
	///
	void draw(Painter &, int, int);
	///
	string label(int row) const;
	///
	void label(int row, string const & label);
	///
	void numbered(int row, bool num);
	///
	bool numbered(int row) const;
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

	///
	void addRow(int);
	///
	void delRow(int);
	///
	void addCol(int);
	///
	void delCol(int);
	///
	void appendRow();

	/// change type
	void mutate(string const &);
	///
	void mutate(short);

private:
	///
	void validate1(LaTeXFeatures & features);
	///
	void header_write(std::ostream &) const;
	///
	void footer_write(std::ostream &) const;
	///
	void glueall();
	///
	string nicelabel(int row) const;

	///
	std::vector<int> nonum_;
	///
	std::vector<string> label_;
};

#endif
