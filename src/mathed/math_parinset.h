// -*- C++ -*-
#ifndef MATH_PARINSET_H
#define MATH_PARINSET_H

#include "math_inset.h"

struct MathedRowSt;


/** The math paragraph base class, base to all editable math objects */
class MathParInset: public MathedInset  {
 public: 
    ///
    MathParInset(short st = LM_ST_TEXT, string const & nm = string(),
		 short ot = LM_OT_MIN);
    ///
    explicit
    MathParInset(MathParInset *);
    ///
    virtual ~MathParInset();
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
    virtual void SetData(MathedArray *);
    ///
    virtual MathedArray * GetData() { return array; }
    /// Paragraph position
    virtual void GetXY(int &, int &) const;
    ///
    virtual void setXY(int x, int y) { xo = x;  yo = y; }
    ///
    virtual void SetFocus(int, int) {}
    ///
    virtual bool Inside(int, int);   
    // Tab stuff used by Matrix.
    ///
    virtual void SetAlign(char, string const &) {}
    ///
    virtual int GetColumns() const { return 1; }
    ///
    virtual int GetRows() const { return 1; }
    ///
    virtual bool isMatrix() const { return false; }
    // Vertical switching
    ///
    virtual bool setArgumentIdx(int i) { return (i == 0); }
    ///
    virtual bool setNextArgIdx() { return false; }
    ///
    virtual int getArgumentIdx() const { return 0; }
    ///
    virtual int getMaxArgumentIdx() const { return 0; }
    ///
    virtual void SetStyle(short);
    ///
    virtual MathedRowSt * getRowSt() const;
    ///
    virtual void setRowSt(MathedRowSt *) {}
    ///
    virtual bool Permit(short f) const { return bool(f & flag); }
 protected:
    /// Paragraph data is stored here
    MathedArray * array;
    /// Cursor start position
    int xo;
    ///
    int yo;
    /// 
    short flag;
 private:
    ///
    virtual void setFlag(MathedParFlag f) { flag |= f; }
    ///
    friend class InsetFormula;
    ///
    friend class MathedXIter;
    ///
    friend class MathedCursor;
    ///
    friend MathedArray * mathed_parse(unsigned flags = 0,
				       MathedArray * a = 0,
				       MathParInset ** p = 0);
};


inline
bool MathParInset::Inside(int x, int y) 
{
  return (x >= xo && x <= xo + width && y <= yo + descent && y >= yo - ascent);
}


inline
void MathParInset::GetXY(int & x, int & y) const
{
   x = xo; y = yo;
}


inline
void MathParInset::UserSetSize(short sz)
{
   if (sz >= 0) {
       size = sz;      
       flag = flag & ~LMPF_FIXED_SIZE;
   }
}


inline
void MathParInset::SetStyle(short sz) 
{
    if (Permit(LMPF_FIXED_SIZE)) {
	if (Permit(LMPF_SCRIPT)) 
	  sz = (sz < LM_ST_SCRIPT) ? LM_ST_SCRIPT: LM_ST_SCRIPTSCRIPT;
	if (Permit(LMPF_SMALLER) && sz < LM_ST_SCRIPTSCRIPT) {
	    ++sz;
	} 
	MathedInset::SetStyle(sz);
    }
}
#endif
