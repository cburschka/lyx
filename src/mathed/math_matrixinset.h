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
	MathMatrixInset(MathInsetTypes t, unsigned int cols);
	///
	MathInset * clone() const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void metrics(MathStyles st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	string label(unsigned int row) const;
	///
	void label(unsigned int row, string const & label);
	///
	void numbered(unsigned int row, bool num);
	///
	bool numbered(unsigned int row) const;
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
	void addRow(unsigned int);
	///
	void delRow(unsigned int);
	///
	void addCol(unsigned int);
	///
	void delCol(unsigned int);
	///
	void appendRow();

	/// change type
	void mutate(string const &);
	///
	void mutate(MathInsetTypes);

	///
	int defaultColSpace(unsigned int col);
	///
	char defaultColAlign(unsigned int col);

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
	string nicelabel(unsigned int row) const;

	///
	MathInsetTypes objtype_;
	///
	std::vector<int> nonum_;
	///
	std::vector<string> label_;
};

#endif
