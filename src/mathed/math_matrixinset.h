// -*- C++ -*-
#ifndef MATH_MATRIXINSET_H
#define MATH_MATRIXINSET_H

#include <vector>

#include "math_parinset.h"

/** Multiline math paragraph base class.
    This is the base to all multiline editable math objects
    like array and eqnarray.
    \author Alejandro Aguilar Sierra
*/
class MathMatrixInset : public MathParInset {
public: 
	///
	explicit
	MathMatrixInset(int m = 1, int n = 1, short st = LM_ST_TEXT);
	///
	explicit
	MathMatrixInset(MathMatrixInset *);
	///
	MathedInset * Clone();
	///
	virtual ~MathMatrixInset();
	///
	void draw(Painter &, int, int);
	///
	void Write(std::ostream &, bool fragile);
	///
	void Metrics();
	///
	void setData(MathedArray const &);
	///
	void SetAlign(char, string const &);
	///
	int GetColumns() const;
	///
	int GetRows() const;
	///
	virtual bool isMatrix() const;
	
	/// Use this to manage the extra information independently of paragraph
	MathedRowSt * getRowSt() const;
	///
	void setRowSt(MathedRowSt * r);
private:
	///  Number of columns & rows
	int nc_;
	///
	int nr_;
	/// tab sizes
	std::vector<int> ws_;   
	/// 
	char v_align_; // add approp. type
	///
	//std::vector<char> h_align;
	string h_align_; // a vector would perhaps be more correct
	/// Vertical structure
	MathedRowSt * row_;
};


inline
int MathMatrixInset::GetColumns() const
{
	return nc_;
}


inline
int MathMatrixInset::GetRows() const
{
	return nr_;
}


inline
bool MathMatrixInset::isMatrix() const
{
	return true;
}
	

inline
MathedRowSt * MathMatrixInset::getRowSt() const
{
	return row_;
}


inline
void MathMatrixInset::setRowSt(MathedRowSt * r)
{
	row_ = r;
}
#endif
