// -*- C++ -*-
/*
 *  File:        math_iter.h
 *  Purpose:     Iterator for Math paragraphs
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1997
 *  Description: Using iterators is the only way to handle math paragraphs 
 *
 *  Dependencies: Xlib
 *
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta, Mathed & Lyx project.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 * 
 */

#ifndef MATH_ITER
#define MATH_ITER

#ifdef __GNUG__
#pragma interface
#endif

#include "math_defs.h"

///
enum mathIterFlags {
    /// Allow newlines
    MthIF_CR = 1,
    /// Allow tabs
    MthIF_Tabs = 2
};


/** Specialized array iterator for math paragraph.
    Used for storing and querying data operations
*/
class MathedIter {
 public:
    ///
    MathedIter() {
	pos = 0;
	fcode = 0;
	array = 0;
	flags = 0;
	ncols = row = col = 0;
    }
    ///
    explicit
    MathedIter(MathedArray *);
    ///
    virtual ~MathedIter() {}
    ///
    bool goNextCode(MathedTextCodes);
    ///
    void goPosRel(int);
    ///
    void goPosAbs(int);
    ///
    int Empty() const { return array->last() <= 1; }
    ///
    int OK() const { return array && (pos < array->last()); }
    ///
    int IsFirst() const { return (pos == 0); }
    ///
    byte GetChar() const;
    ///
    string const GetString() const;
    ///
    MathedInset * GetInset() const;
    ///
    MathParInset * GetActiveInset() const;
    ///
    bool IsInset() const;
    ///
    bool IsActive() const;
    ///
    bool IsFont() const;
    ///
    bool IsScript() const;
    ///
    bool IsTab() const;
    ///
    bool IsCR() const;
    ///
    virtual void Reset();
    ///
    virtual void Insert(byte, MathedTextCodes c = LM_TC_CONST);
    ///
    virtual void Insert(MathedInset *, int t = LM_TC_INSET);
    ///
    virtual bool Delete();
    ///
    virtual bool Next();
    /// Check consistency of tabs and newlines
    void checkTabs();
    /// Try to adjust tabs in the expected place, as in eqnarrays
    void adjustTabs();
    ///
    short FCode() const { return fcode; }
    ///
    int getPos() const { return pos; }
    ///
    int getRow() const { return row; }
    ///
    int getCol() const { return col; }
    ///
    void setNumCols(int n) { ncols = n; }
    ///
    void SetData(MathedArray * a) { array = a; Reset(); }
    ///
    MathedArray * GetData() const { return array; }
    
    /// Copy every object from position p1 to p2
    MathedArray * Copy(int p1 = 0, int p2 = 10000);
   
    /// Delete every object from position p1 to p2
    void Clear();
    
 protected:
    ///
    void split(int);
    ///
    void join(int);
    ///
    int flags;
    ///
    mutable short fcode;
    ///
    mutable int pos;
    ///
    int row, col, ncols;
    ///
    MathedArray * array;
    // one element stack
    struct MIState {
	///
	short fcode;
	///
	int x, y;
	///
	int pos, row, col;
    };
    ///
    MIState stck;
    /// Saves the current state of the iterator
    virtual void ipush();
    /// Recover previous state
    virtual void ipop();
};

///
#define MX_WAS_SUB   1
///
#define MX_WAS_SUPER 2


/**
 A graphic iterator (updates position.) Used for
 metrics and updating cursor position
 */
class MathedXIter: public MathedIter {
 public:
    ///
    MathedXIter()
	    : MathedIter(), sx(0), sw(0) {
	    x = y = size = 0;  p = 0; crow = 0;
    }
    //
    MathedXIter(MathParInset *);
    ///
    void SetData(MathParInset *);
    ///
    MathParInset * getPar() const { return p; }
    ///
    bool Next();
    ///
    bool Prev();
    ///
    bool Up();
    ///
    bool Down();
    ///
    bool goNextColumn();
    ///
    void GoLast();
    ///
    void GoBegin();
    ///
    void Adjust();
    ///
    inline
    void GetPos(int &, int &) const;
    ///
    inline
    void GetIncPos(int &, int &) const;
    ///
    string const GetString() const;
    ///
    int GetX() const;
    ///
    int GetY() const;
    ///
    void subMetrics(int, int);
    ///
    void fitCoord(int, int);
    /// 
    void getAD(int & a, int & d) const;
    
    /// Create a new row and insert #ncols# tabs.
    void addRow();
    ///
    void delRow();
    
    ///
    bool setLabel(string const & label);
    ///
    static string error_label;
    ///
    string const & getLabel() const {
	    return crow ? crow->getLabel() : error_label;
    }
    ///
    bool setNumbered(bool);
	
    ///
    void setTab(int, int);
    /// Merge the array at current position
    void Merge(MathedArray *);
    /// Delete every object from current position to pos2
    void Clean(int pos2);
    ///
    MathedRowSt * adjustVerticalSt();
    
private:
    /// This function is not recursive, as MathPar::Metrics is
    void IMetrics(int, int &, int &, int &);
    /// Font size (display, text, script, script2) 
    int size;
    /// current position
    mutable int x;
    ///
    int y;
    ///
    MathParInset * p;
    
    // Limits auxiliary variables
    /// Position and max width of a script
    int sx, sw;
    /// true= center, false= left align (default)
    bool limits;
    /// Type of previous script
    short s_type;  
    ///
    void ipush();
    ///
    void ipop();

protected:
    /// 
    MathedRowSt * crow;
    
private:
    ///
    friend class MathedCursor;
};


//--------------------   Inline functions   --------------------------//


inline
bool MathedIter::IsInset() const
{
    return MathIsInset((*array)[pos]);
}
 
inline
bool MathedIter::IsActive() const
{
    return MathIsActive((*array)[pos]);
}

inline
bool MathedIter::IsFont() const
{
    return MathIsFont((*array)[pos]);
}


inline
bool MathedIter::IsScript() const
{
    return MathIsScript((*array)[pos]);
}   

inline
bool MathedIter::IsTab() const
{
    return ((*array)[pos] == LM_TC_TAB);
}  


inline
bool MathedIter::IsCR() const
{
    return ((*array)[pos] == LM_TC_CR);
}  


inline
MathedIter::MathedIter(MathedArray * d)
	: array(d)
{
    pos = 0;
    row = col = 0;
    fcode = (array && IsFont()) ? (*array)[0]: 0;
}


inline
void MathedIter::ipush()
{ 
    stck.fcode = fcode;
    stck.pos = pos;
    stck.row = row;
    stck.col = col; 
}


inline
void MathedIter::ipop()
{ 
    fcode = stck.fcode;
    pos = stck.pos;
    row = stck.row;
    col = stck.col;  
}


inline
void MathedXIter::GetPos(int & xx, int & yy) const
{ 
    if (p) 
      p->GetXY(xx, yy);
    else {
	    xx = 0;
	    yy = 0;
    }        
    xx += x;  yy += y;
}


inline 
int MathedXIter::GetX() const
{ 
    int xx;
    int dummy_y;
    GetPos(xx, dummy_y);
    return xx; 
}


inline 
int MathedXIter::GetY() const
{ 
    int dummy_x;
    int yy;
    GetPos(dummy_x, yy);
    return yy; 
}


inline
void MathedXIter::GetIncPos(int & xx, int & yy) const
{ 
    xx = x;
    yy = y; 
}


inline
void MathedXIter::getAD(int & a, int & d) const
{ 
    if (crow) {
	a = crow->ascent();
	d = crow->descent();
    } else 
      if (p) {
	  a = p->Ascent();
	  d = p->Descent();
      } else {
	  a = d = 0;
      }
}


#endif

