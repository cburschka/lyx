// -*- C++ -*-
#ifndef MATH_PARINSET_H
#define MATH_PARINSET_H

#include "math_inset.h"
#include "math_defs.h"

struct MathedRowSt;
class MathedArray;


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
    virtual MathedArray * GetData();
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
    virtual bool Permit(short f) const;
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
    virtual void setFlag(MathedParFlag f);
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


#endif
