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
 *  Copyright: (c) 1996, 1997 Alejandro Aguilar Sierra
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

#include <cstdio>
#include "LString.h"

#include "array.h"

///
#define MATH_ALIGN_LEFT    1
///
#define MATH_ALIGN_RIGHT   2
///
#define MATH_ALIGN_BOTTOM  4
///
#define MATH_ALIGN_TOP     8
///
#define MATH_COLSEP 8
///
#define MATH_ROWSEP 8


/// Standard Math Sizes (Math mode styles)
enum MathedStyles {
	///
   LM_ST_DISPLAY=0,
   ///
   LM_ST_TEXT,
   ///
   LM_ST_SCRIPT,
   ///
   LM_ST_SCRIPTSCRIPT
};


/// Standard LaTeX Math Environments
enum MathedEnvironment {
	///
   LM_EN_INTEXT=0,
   ///
   LM_EN_DISPLAY,
   ///
   LM_EN_EQUATION,
   ///
   LM_EN_EQNARRAYAST,
   ///
   LM_EN_EQNARRAY,
   ///
   LM_EN_ARRAY
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

ostream & operator<<(ostream &, MathedTextCodes mtc);

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

class LString;
class MathedInset;
class MathParInset;


/** Abstract base class for all math objects.
    A math insets is for use of the math editor only, it isn't a
    general LyX inset. It's used to represent all the math objects.
    The formulaInset (a LyX inset) encapsulates a math inset.
 */
class MathedInset  {
 public: 
    /// A math inset has a name (usually its LaTeX name), type and font-size
    MathedInset(char const *nm, short ot, short st);
    ///
    MathedInset(MathedInset*);
    ///
    virtual ~MathedInset() { };
    
    /// Draw the object 
    virtual void Draw(int x, int baseline)=0;
    
    /// Write LaTeX and Lyx code
    virtual void Write(FILE *file)=0;
    /// Write LaTeX and Lyx code
    virtual void Write(string & file)=0;
   
    /// Reproduces itself
    virtual MathedInset *Clone()=0;
   
    /// Compute the size of the object
    virtual void Metrics()=0; 
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
    virtual void SetLimits(bool) { }   
   
    ///
    char const *GetName() const { return name; }
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
    virtual void  SetName(char const* n) { name = n; }
    /// 
    void setDrawable(long unsigned int d) { pm = d; }
 
 protected:
    ///
    char const *name;
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
    /// This works while only one process can draw unless
    /// the process have their own data
    static long unsigned int pm;
    /// Default metrics
    static int df_asc, df_des, df_width;

    /// In a near future maybe we use a better fonts renderer than X
    void drawStr(short, int, int, int, byte*, int);
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
    MathParInset(short st=LM_ST_TEXT, char const *nm=0, short ot=LM_OT_MIN);
    ///
    MathParInset(MathParInset*);
    ///
    virtual ~MathParInset();
    ///
    virtual MathedInset *Clone();   

    /// Draw the object on a drawable
    virtual void Draw(int x, int baseline);
   
    /// Write LaTeX code
    virtual void Write(FILE *file);
    /// Write LaTeX code
    virtual void Write(string &file);
    ///
    virtual void Metrics();
    ///
    virtual void UserSetSize(short);
 
    /// Data is stored in a LyXArray
    virtual void SetData(LyxArrayBase *);
    ///
    virtual LyxArrayBase * GetData() { return array; }

    /// Paragraph position
    virtual void GetXY(int&, int&) const;
    ///
    virtual void setXY(int x, int y) { xo =x;  yo = y; }
    ///
    virtual void SetFocus(int,int) { };
    ///
    virtual bool Inside(int, int);   
   
    // Tab stuff used by Matrix.
    ///
    virtual void SetAlign(char, char const*) { };
//    ///
//    virtual int GetTabPos() { return 0; }
//    ///
//    virtual int GetTab(int) { return 0; }
    ///
    virtual int GetColumns() { return 1; }
    ///
    virtual int GetRows() { return 1; }
    ///
    virtual bool isMatrix() { return false; }
//    /// These functions should report an error
//    virtual char const* GetLabel() { return 0; }
//    virtual char const* GetLabel(int) { return 0; }

    // Vertical switching
    ///
    virtual bool setArgumentIdx(int i) { return (i==0); }
    ///
    virtual bool setNextArgIdx() { return false; }
    ///
    virtual int  getArgumentIdx() { return 0; }
    ///
    virtual int  getMaxArgumentIdx() { return 0; }
//    ///
//    virtual void SetLabel(char const*) { }
    ///
    virtual void SetStyle(short);
	///
    virtual MathedRowSt *getRowSt() const { return 0; }
	///
    virtual void setRowSt(MathedRowSt*) { }
	///
    virtual bool Permit(short f) { return (bool)(f & flag); }
    
 protected:
    /// Paragraph data is stored here
    LyxArrayBase *array;
    /// Cursor start position
    int xo, yo;
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
    friend LyxArrayBase *mathed_parse(unsigned flags = 0, LyxArrayBase*a=0, MathParInset**p=0);
};


/* The physical structure of a row and aditional information is stored here.
    It allows to manage the extra info independently of the paragraph data.  
    Only used for multiline paragraphs.
 */
struct MathedRowSt {    
    /// 
    MathedRowSt(int n) {
	    w = new int[n+1]; // this leaks
	next = 0;
	label = 0;
	numbered = true;
    }
    ///
    ~MathedRowSt() {
	delete[] w;
	if (label) delete[] label;
    }
    /// Should be const but...
    MathedRowSt* getNext() const  { return next; }
    /// ...we couldn't use this.
    void setNext(MathedRowSt* n) { next = n; }
    ///
    char const* getLabel() const { return label; }
    ///
    bool isNumbered() const { return numbered; }
    ///
    int  getBaseline() const { return y; }    
    ///
    int  getTab(int i) { return w[i]; }
    /// 
    void setLabel(char* l) { label = l; }
    ///
    void setNumbered(bool nf) { numbered = nf; }
    ///
    void setTab(int i, int t) { w[i] = t; }
    
 private:
    /// Vericals 
    int asc, desc, y;
    /// widths 
    int *w;
    /// 
    char *label;
    ///
    bool numbered;
    ///
    MathedRowSt *next;
	///
    friend class MathMatrixInset;
	///
    friend class MathedXIter;
};


/** Multiline math paragraph base class.
    This is the base to all multiline editable math objects
    like array and eqnarray. 
 */
class MathMatrixInset: public MathParInset {
 public: 
    ///
    MathMatrixInset(int m=1, int n=1, short st=LM_ST_TEXT);
    ///
    MathMatrixInset(MathMatrixInset*);
    ///
    MathedInset *Clone();
    ///
    virtual ~MathMatrixInset();
    ///
    void Draw(int, int);
    ///
    void Write(FILE *file);
    ///
    void Write(string &file);
    ///
    void Metrics();
    ///
    void SetData(LyxArrayBase *);
    ///
    void SetAlign(char, char const*);
    ///
    char *GetAlign(char* vv) {
	*vv = v_align;
	return h_align;
    }
//    ///
//    int GetTab(int);
    ///
    int GetColumns() { return nc; }
    ///
    int GetRows() { return nr; }
    ///
    virtual bool isMatrix() { return true; }

    /// Use this to manage the extra information independently of paragraph
    MathedRowSt *getRowSt() const { return row; }
	///
    void setRowSt(MathedRowSt* r) { row = r; }
    
 protected:
    ///  Number of columns & rows
    int nc, nr;
    /// tab sizes
    int *ws;   
    /// 
    char v_align; // add approp. signedness
    ///
    char* h_align;
    /// Vertical structure
    MathedRowSt *row;

};



/*************************  Prototypes  **********************************/
/// 
LyxArrayBase *mathed_parse(unsigned flags, LyxArrayBase *data, MathParInset **mt);
///
void mathed_write(MathParInset*, FILE *, int*, char fragile, char const* label=0);
///
void mathed_write(MathParInset*, string&, int*, char fragile, char const* label=0);
///
void mathed_parser_file(FILE*, int);
///
int mathed_parser_lineno();
///
int MathedLookupBOP(short);

/************************ Inline functions ********************************/

///
#define MathIsInset(x)  (LM_TC_INSET<=(x) && (x)<=LM_TC_ACTIVE_INSET)
///
#define MathIsFont(x)  (LM_TC_CONST<=(x) && (x)<=LM_TC_BSYM)
///
#define MathIsAlphaFont(x)  (LM_TC_VAR<=(x) && (x)<=LM_TC_TEXTRM)
///
#define MathIsActive(x)  (LM_TC_INSET<(x) && (x)<=LM_TC_ACTIVE_INSET) 
///
#define MathIsUp(x)    ((x)==LM_TC_UP) 
///
#define MathIsDown(x)  ((x)==LM_TC_DOWN)
///
#define MathIsScript(x)  ((x)==LM_TC_DOWN || (x)==LM_TC_UP)  
///
#define MathIsBOPS(x)    (MathedLookupBOP(x)>LMB_NONE)


///
inline bool MathIsBinary(short x)
{
    return (x==LM_TC_BOP || x==LM_TC_BOPS);
}

///
inline bool MathIsSymbol(short x) {
    return (LM_TC_SYMB<=x && x<=LM_TC_BSYM);
}
     

inline 
MathedInset::MathedInset(char const *nm, short ot, short st):
  name(nm), objtype(ot), size(st) 
{
   width = ascent = descent = 0;
}

inline
bool MathParInset::Inside(int x, int y) 
{
  return (x>=xo && x<=xo+width && y<=yo+descent && y>=yo-ascent);
}


inline
void MathParInset::GetXY(int& x, int& y) const
{
   x = xo; y = yo;
}


inline
void MathParInset::UserSetSize(short sz)
{
   if (sz>=0) {
       size = sz;      
       flag = flag & ~LMPF_FIXED_SIZE;
   }
}


inline
void MathParInset::SetStyle(short sz) 
{
    if (Permit(LMPF_FIXED_SIZE)) {
	if (Permit(LMPF_SCRIPT)) 
	  sz = (sz<LM_ST_SCRIPT) ? LM_ST_SCRIPT: LM_ST_SCRIPTSCRIPT;
	if (Permit(LMPF_SMALLER) && sz < LM_ST_SCRIPTSCRIPT) {
	    sz++;
	} 
	MathedInset::SetStyle(sz);
    }
}

#endif

