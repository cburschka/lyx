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


/**
 Specialized array iterator for amth paragraph.  Used for
 storing and querying data operations
 */
class MathedIter {
 public:
    ///
    MathedIter()
    {
	pos = 0;
	fcode = 0;
	array = 0;
	flags = 0;
	ncols = row = col = 0;
    }
    ///
	explicit
    MathedIter(LyxArrayBase *);
    ///
    virtual ~MathedIter() { }
    ///
    bool goNextCode(MathedTextCodes);
    ///
   void goPosRel(int);
    ///
    void goPosAbs(int);
    ///
    int Empty() { return array->Last()<= 1; }
    ///
    int OK() { return array && (pos < array->Last()); }
    ///
    int IsFirst() { return (pos == 0); }
    ///
    byte GetChar();
    ///
    byte * GetString(int& len);
    ///
    MathedInset * GetInset();
    ///
    MathParInset * GetActiveInset();
    ///
    bool IsInset();
    ///
    bool IsActive();
    ///
    bool IsFont();
    ///
    bool IsScript();
    ///
    bool IsTab();
    ///
    bool IsCR();
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
    void SetData(LyxArrayBase * a) { array = a; Reset(); }
    ///
    LyxArrayBase * GetData() { return array; }
    
    /// Copy every object from position p1 to p2
    LyxArrayBase * Copy(int p1= 0, int p2= 10000);
   
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
    short fcode;
    ///
    int pos;
    ///
    int row, col, ncols;
    ///
    LyxArrayBase *array;
    // one element stack
    struct MIState {
	    ///
	short fcode;
	    ///
	int x, y;
	    ///
	int pos, row, col;
    } stck;
    
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
    MathedXIter() : MathedIter(), sx(0), sw(0) { x = y = size = 0;  p = 0; crow = 0; }
    ///
    MathedXIter(MathParInset*);
    ///
    ~MathedXIter() { };
    ///
    void SetData(MathParInset *);
    ///
    MathParInset *getPar() { return p; }
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
    void GetPos(int&, int&);
    ///
    inline
    void GetIncPos(int&, int&);
    ///
    byte* GetString(int&);
    ///
    int GetX();
    ///
    int GetY();
    ///
    void subMetrics(int, int);
    ///
    void fitCoord(int, int);
    /// 
    void getAD(int& a, int& d);
    
    /// Create a new row and insert #ncols# tabs.
    void addRow();
	///
	void delRow();
    
	/**$ These two functions will be moved from here */
	  //@{
	///
    bool setLabel(char* label);
    ///
    char const * getLabel() const {
	    return crow->getLabel();
    }
	///
    bool setNumbered(bool);
	//@}
	
	///
    void setTab(int, int);
    /// Merge the array at current position
    void Merge(LyxArrayBase*);
    /// Delete every object from current position to pos2
    void Clean(int pos2);
    MathedRowSt *adjustVerticalSt();
    
 private:
    /// This function is not recursive, as MathPar::Metrics is
    void IMetrics(int, int&, int&, int&);
    /// Font size (display, text, script, script2) 
    int size;
    /// current position
    int x, y;
//    /// max ascent and descent
//    int asc, des;
	///
    MathParInset *p;
    
    // Limits auxiliary variables
    /// Position and max width of a script
    int sx, sw;
    /// true= center, false= left align (default)
    bool limits;
    /// Type of previous script
    short s_type;  

    void ipush();

    void ipop();

 protected:
    /// 
    MathedRowSt *crow;
    
 private:
    ///
    friend class MathedCursor;
};


//--------------------   Inline functions   --------------------------//


inline
bool MathedIter::IsInset()
{
    return MathIsInset((*array)[pos]);
}
 
inline
bool MathedIter::IsActive()
{
    return MathIsActive((*array)[pos]);
}

inline
bool MathedIter::IsFont()
{
    return MathIsFont((*array)[pos]);
}


inline
bool MathedIter::IsScript()
{
    return MathIsScript((*array)[pos]);
}   

inline
bool MathedIter::IsTab()
{
    return ((*array)[pos] == LM_TC_TAB);
}  


inline
bool MathedIter::IsCR()
{
    return ((*array)[pos] == LM_TC_CR);
}  


inline
MathedIter::MathedIter(LyxArrayBase * d): array(d)
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
void MathedXIter::GetPos(int&xx, int& yy)
{ 
    if (p) 
      p->GetXY(xx, yy);
    else
      { xx = 0;  yy = 0; }        
    xx += x;  yy += y;
}

inline 
int MathedXIter::GetX() 
{ 
    int xx, yy;
    GetPos(xx, yy);
    return xx; 
}

inline 
int MathedXIter::GetY() 
{ 
    int xx, yy;
    GetPos(xx, yy);
    return yy; 
}


inline
void MathedXIter::GetIncPos(int& xx, int& yy) 
{ 
    xx = x;  yy = y; 
}


inline
void MathedXIter::getAD(int& a, int& d) 
{ 
    if (crow) {
	a = crow->asc;
	d = crow->desc;
    } else 
      if (p) {
	  a = p->Ascent();
	  d = p->Descent();
      } else {
	  a = d = 0;
      }
}


#endif

