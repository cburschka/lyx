// -*- C++ -*-
#ifndef MATH_PARINSET_H
#define MATH_PARINSET_H

#include "array.h"
#include "math_inset.h"
#include "math_defs.h"
#include "math_rowst.h"

#ifdef __GNUG__
#pragma interface
#endif

class MathedArray;


/** The math paragraph base class, base to all editable math objects
    \author Alejandro Aguilar Sierra
 */
class MathParInset : public MathedInset  {
public: 
	///
	MathParInset(short st = LM_ST_TEXT, string const & nm = string(),
		     short ot = LM_OT_MIN);
	///
	virtual MathedInset * Clone();
	/// Draw the object on a drawable
	virtual void draw(Painter &, int x, int baseline);
	/// Write LaTeX code
	virtual void Write(std::ostream &, bool fragile);
	///
	virtual void Metrics();
	///
	virtual void UserSetSize(short);
	/// Data is stored in a LyXArray
	virtual void setData(MathedArray const &);
	///
	virtual MathedArray & GetData();
	///
	virtual MathedArray const & GetData() const;
	/// Paragraph position
	virtual void GetXY(int &, int &) const;
	///
	virtual void setXY(int x, int y);
	///
	virtual void SetFocus(int, int) {}
	///
	virtual bool Inside(int, int);   
	// Tab stuff used by Matrix.
	///
	virtual void SetAlign(char, string const &);
	///
	virtual int GetColumns() const;
	///
	virtual int GetRows() const;
	///
	virtual bool isMatrix() const;
	// Vertical switching
	///
	virtual bool setArgumentIdx(int i);
	///
	virtual bool setNextArgIdx();
	///
	virtual int getArgumentIdx() const;
	///
	virtual int getMaxArgumentIdx() const;
	///
	virtual void SetStyle(short);
	///
	virtual MathedRowContainer & getRowSt();
	///
	virtual void setRowSt(MathedRowContainer &);
	///
	virtual bool Permit(short f) const;
	///
	int xo() const;
	///
	int yo() const;
	///
	void clear();
protected:
	/// Paragraph data is stored here
	MathedArray array;
	/// 
	short flag;
	///
	void xo(int tx);
	///
	void yo(int ty);
private:
	/// Cursor start position
	int xo_;
	///
	int yo_;
};


inline
void MathParInset::SetAlign(char, string const &)
{}


inline
int MathParInset::GetColumns() const
{
	return 1;
}


inline
int MathParInset::GetRows() const
{
	return 1;
}


inline
bool MathParInset::isMatrix() const
{
	return false;
}


inline
bool MathParInset::setArgumentIdx(int i)
{
	return (i == 0);
}


inline
bool MathParInset::setNextArgIdx()
{
	return false;
}


inline
int MathParInset::getArgumentIdx() const
{
	return 0;
}


inline
int MathParInset::getMaxArgumentIdx() const
{
	return 0;
}


inline
void MathParInset::setRowSt(MathedRowContainer &)
{}


inline
int MathParInset::xo() const
{
	return xo_;
}


inline
int MathParInset::yo() const
{
	return yo_;
}


inline
void MathParInset::xo(int tx)
{
	xo_ = tx;
}


inline
void MathParInset::yo(int ty)
{
	yo_ = ty;
}
#endif
