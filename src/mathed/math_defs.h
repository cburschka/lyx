// -*- C++ -*-
/*
 *  File:        math_defs.h
 *  Purpose:     Math editor definitions 
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Math paragraph and objects for a WYSIWYG math editor.
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

#ifndef MATH_DEFS
#define MATH_DEFS

#ifdef __GNUG__
#pragma interface
#endif

#include <iosfwd>

#include "LString.h"
#include "debug.h"

#include "array.h"

class Painter;

///
enum math_align {
	///
	MATH_ALIGN_LEFT = 1,
	///
	MATH_ALIGN_RIGHT = 2,
	///
	MATH_ALIGN_BOTTOM = 4,
	///
	MATH_ALIGN_TOP = 8
};
///
static int const MATH_COLSEP = 8;
///
static int const MATH_ROWSEP = 8;


/// Standard Math Sizes (Math mode styles)
enum MathedStyles {
	///
	LM_ST_DISPLAY = 0,
	///
	LM_ST_TEXT,
	///
	LM_ST_SCRIPT,
	///
	LM_ST_SCRIPTSCRIPT
};


/** The restrictions of a standard LaTeX math paragraph
  allows to get a small number of text codes (<30) */
enum MathedTextCodes  {
	/// This must be >= 0
	LM_TC_MIN = 0,
	/// Open and Close group
	LM_TC_OPEN,
	///
	LM_TC_CLOSE,
	/// Tabulator
	LM_TC_TAB,
	/// New line
	LM_TC_CR,
	/// Math Inset
	LM_TC_INSET,
	/// Super and sub scripts
	LM_TC_UP,
	///
	LM_TC_DOWN,
	/// Editable Math Inset
	LM_TC_ACTIVE_INSET,
	/// Editable Text Inset
	LM_TC_TEXT_INSET,
	///
	LM_FONT_BEGIN,
	/// Internal code for constants
	LM_TC_CONST,
	/// Internal code for variables
	LM_TC_VAR,
	///
	LM_TC_RM,
	///
	LM_TC_CAL,
	///
	LM_TC_BF,
	///
	LM_TC_SF,
	///
	LM_TC_TT,
	///
	LM_TC_IT,
	///
	LM_TC_TEXTRM,
	/// Math mode TeX characters ",;:{}"
	LM_TC_TEX,
	/// Special characters "{}&#_%"
	LM_TC_SPECIAL,
	/// Internal code for operators
	LM_TC_BOP,
	/// Internal code for symbols
	LM_TC_SYMB,
	///
	LM_TC_BOPS,
	///
	LM_TC_BSYM,
	///
	LM_FONT_END,
	
	/// This must be < 32 
	LM_TC_MAX
};

///
std::ostream & operator<<(std::ostream &, MathedTextCodes mtc);

///
#define LM_TC_NORMAL LM_TC_VAR
 
       
/// Types of lyx-math insets 
enum MathedInsetTypes  {
	///
	LM_OT_MIN = 0,
	/// A simple paragraph
	LM_OT_PAR,
	/// A simple numbered paragraph
	LM_OT_PARN,
	/// A multiline paragraph
	LM_OT_MPAR,
	/// A multiline numbered paragraph
	LM_OT_MPARN,
	///
	LM_OT_ALIGNAT,
	///
	LM_OT_ALIGNATN,
	///
	LM_OT_MULTLINE,
	///
	LM_OT_MULTLINEN,
	/// An array
	LM_OT_MATRIX,

	/// A big operator
	LM_OT_BIGOP,
	/// A LaTeX macro
	LM_OT_UNDEF,
	///
	LM_OT_FUNC,
	///
	LM_OT_FUNCLIM,
	///
	LM_OT_SCRIPT,
	///
	LM_OT_SPACE,
	///
	LM_OT_DOTS,
	/// A fraction
	LM_OT_FRAC,
	///
	LM_OT_ATOP,
	///
	LM_OT_STACKREL,
	/// A radical
	LM_OT_SQRT,
	/// A delimiter
	LM_OT_DELIM,
	/// A decoration
	LM_OT_DECO,
	/// An accent
	LM_OT_ACCENT,
	///
	LM_OT_MACRO,
	///
	LM_OT_MACRO_ARG,
	///
	LM_OT_MAX
};

///
enum MathedBinaryTypes {
	///
	LMB_NONE = 0,
	///
	LMB_RELATION,
	///
	LMB_OPERATOR,
	///
	LMB_BOP = (LMB_RELATION | LMB_OPERATOR)
};

class MathParInset;

/** Abstract base class for all math objects.
    A math insets is for use of the math editor only, it isn't a
    general LyX inset. It's used to represent all the math objects.
    The formulaInset (a LyX inset) encapsulates a math inset.
 */
class MathedInset  {
 public: 
    /// A math inset has a name (usually its LaTeX name), type and font-size
    MathedInset(string const & nm, short ot, short st);
    ///
    explicit
    MathedInset(MathedInset *);
    ///
    virtual ~MathedInset() {}

    /// Draw the object
    virtual void draw(Painter &, int x, int baseline) = 0;	

    /// Write LaTeX and Lyx code
    virtual void Write(std::ostream &, bool fragile) = 0;

    /// Reproduces itself
    virtual MathedInset * Clone() = 0;
   
    /// Compute the size of the object
    virtual void Metrics() = 0; 
    /// 
    virtual int Ascent() const { return ascent; }
    ///
    virtual int Descent() const { return descent; }
    ///
    virtual int Width() const { return width; }
    ///
    virtual int Height() const { return ascent + descent; }
    
    ///
    virtual bool GetLimits() const { return false; }
    ///
    virtual void SetLimits(bool) {}   
   
    ///
    string const & GetName() const { return name; }
    ///
    short GetType() const { return objtype; }
    ///
    short GetStyle() const { return size; }
          
    //Man:  Avoid to use these functions if it's not strictly necessary 
    ///
    virtual void  SetType(short t) { objtype = t; }
    ///
    virtual void  SetStyle(short st) { size = st; } // Metrics();
    ///
    virtual void  SetName(string const & n) { name = n; }
 protected:
    ///
    string name;
    ///
    short objtype;
    ///
    int width;
    ///
    int ascent;
    ///
    int descent;
    ///
    short size;
    /// Default metrics
    static int df_asc;
    ///
    static int df_des;
    ///
    static int df_width;
    /// In a near future maybe we use a better fonts renderer than X
    void drawStr(Painter &, short, int, int, int, string const &);
    ///
    friend class MathedCursor;
    ///
    friend void mathed_init_fonts();
};

struct MathedRowSt;


/// Paragraph permissions
enum MathedParFlag {
	LMPF_BASIC = 0,
	/// If false can use a non-standard size
	LMPF_FIXED_SIZE = 1,
	/// If true can insert newlines 
	LMPF_ALLOW_CR  = 2,
	/// If true can use tabs
	LMPF_ALLOW_TAB = 4,
	/// If true can insert new columns
	LMPF_ALLOW_NEW_COL = 8,
	/// Smaller than current size (frac)
	LMPF_SMALLER = 16,
	/// Script size (subscript, stackrel)
	LMPF_SCRIPT = 32
};


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
    virtual void SetData(LyxArrayBase *);
    ///
    virtual LyxArrayBase * GetData() { return array; }

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
    virtual MathedRowSt * getRowSt() const { return 0; }
    ///
    virtual void setRowSt(MathedRowSt *) {}
    ///
    virtual bool Permit(short f) { return bool(f & flag); }
    
 protected:
    /// Paragraph data is stored here
    LyxArrayBase * array;
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
    friend LyxArrayBase * mathed_parse(unsigned flags = 0,
				       LyxArrayBase * a = 0,
				       MathParInset ** p = 0);
};


/** The physical structure of a row and aditional information is stored here.
    It allows to manage the extra info independently of the paragraph data.  
    Only used for multiline paragraphs.
 */
struct MathedRowSt {    
    ///
    explicit
    MathedRowSt(int n) {
	    w = new int[n + 1]; // this leaks
	    asc = desc = y = 0;
	    for (int i = 0 ; i < n + 1 ; ++i)
	      w[i] = 0;
	    next = 0;
	    numbered = true;
    }
    ///
    ~MathedRowSt() {
	    delete[] w;
    }
    /// Should be const but...
    MathedRowSt * getNext() const  { return next; }
    /// ...we couldn't use this.
    void setNext(MathedRowSt * n) { next = n; }
    ///
    string const & getLabel() const { return label; }
    ///
    bool isNumbered() const { return numbered; }
    ///
    int  getBaseline() const { return y; }
    ///
    void setBaseline(int b) { y = b; }
    ///
    int ascent() const { return asc; }
    ///
    int descent() const { return desc; }
    ///
    void ascent(int a) { asc = a; }
    ///
    void descent(int d) { desc = d; }
    ///
    int  getTab(int i) const { return w[i]; }
    /// 
    void setLabel(string const & l) { label = l; }
    ///
    void setNumbered(bool nf) { numbered = nf; }
    ///
    void setTab(int i, int t) { w[i] = t; }
    
 private:
    /// Vericals 
    int asc, desc, y;
    /// widths 
    int * w;
    /// 
    string label;
    ///
    bool numbered;
    ///
    MathedRowSt * next;
};


/** Multiline math paragraph base class.
    This is the base to all multiline editable math objects
    like array and eqnarray. 
 */
class MathMatrixInset: public MathParInset {
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
    void SetData(LyxArrayBase *);
    ///
    void SetAlign(char, string const &);
    ///
    int GetColumns() const { return nc; }
    ///
    int GetRows() const { return nr; }
    ///
    virtual bool isMatrix() const { return true; }

    /// Use this to manage the extra information independently of paragraph
    MathedRowSt * getRowSt() const { return row; }
    ///
    void setRowSt(MathedRowSt * r) { row = r; }
    
 protected:
    ///  Number of columns & rows
    int nc;
    ///
    int nr;
    /// tab sizes
    int * ws;   
    /// 
    char v_align; // add approp. type
    ///
    string h_align; // a vector would perhaps be more correct
    /// Vertical structure
    MathedRowSt * row;

};



/*************************  Prototypes  **********************************/
/// 
LyxArrayBase * mathed_parse(unsigned flags, LyxArrayBase * data,
			    MathParInset ** mt);
///
void mathed_write(MathParInset *, std::ostream &, int *, bool fragile,
		  string const & label = string());

///
void mathed_parser_file(std::istream &, int);
///
int mathed_parser_lineno();
///
int MathedLookupBOP(short);

/************************ Inline functions ********************************/

///
inline
bool MathIsInset(short x)
{
	return LM_TC_INSET <= x && x <= LM_TC_ACTIVE_INSET;
}

///
inline
bool MathIsFont(short x)
{
	return LM_TC_CONST <= x && x <= LM_TC_BSYM;
}

///
inline
bool MathIsAlphaFont(short x)
{
	return LM_TC_VAR <= x && x <= LM_TC_TEXTRM;
}

///
inline
bool MathIsActive(short x)
{
	return LM_TC_INSET < x && x <= LM_TC_ACTIVE_INSET;
}

///
inline
bool MathIsUp(short x)
{
	return x == LM_TC_UP;
}

///
inline
bool MathIsDown(short x)
{
	return x == LM_TC_DOWN;
}

///
inline
bool MathIsScript(short x)
{
	return x == LM_TC_DOWN || x == LM_TC_UP;
}

///
inline
bool MathIsBOPS(short x)
{
	return MathedLookupBOP(x) > LMB_NONE;
}

///
inline
bool MathIsBinary(short x)
{
    return x == LM_TC_BOP || x == LM_TC_BOPS;
}

///
inline
bool MathIsSymbol(short x) {
    return LM_TC_SYMB <= x && x <= LM_TC_BSYM;
}
     

inline
MathedInset::MathedInset(string const & nm, short ot, short st):
  name(nm), objtype(ot), size(st) 
{
   width = ascent = descent = 0;
}


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

inline
bool is_eqn_type(short int type)
{
	return type >= LM_OT_MIN && type < LM_OT_MATRIX;
}


inline
bool is_matrix_type(short int type)
{
	return type == LM_OT_MATRIX;
}

inline
bool is_multiline(short int type)
{
	return type >= LM_OT_MPAR && type < LM_OT_MATRIX;
}


inline bool is_ams(short int type)
{
	return type > LM_OT_MPARN && type < LM_OT_MATRIX;
}

inline
bool is_singlely_numbered(short int type)
{
	return type == LM_OT_PARN || type == LM_OT_MULTLINEN;
}

inline
bool is_multi_numbered(short int type)
{
	return type == LM_OT_MPARN || type == LM_OT_ALIGNATN;
}

inline
bool is_numbered(short int type)
{
	return is_singlely_numbered(type) || is_multi_numbered(type);
}

inline
bool is_multicolumn(short int type)
{
	return type == LM_OT_ALIGNAT || type == LM_OT_ALIGNATN;
}

#endif
