// -*- C++ -*-
/*
 *  File:        math_inset.h
 *  Purpose:     Declaration of insets for mathed 
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Math paragraph and objects for a WYSIWYG math editor.
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: (c) 1996, 1997 Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta, Mathed & Lyx project.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

//  Note: These math insets are internal to Mathed and are not derived
//        from lyx inset.

#ifndef MATH_INSET
#define MATH_INSET

#ifdef __GNUG__
#pragma interface
#endif

#include "math_defs.h"
#include "symbol_def.h"
#include "LString.h"


/**
 Functions or LaTeX names for objects that I don't know how to draw.
 */
class MathFuncInset: public MathedInset  {
public:
	///
	MathFuncInset(char const * nm,
		      short ot = LM_OT_FUNC, short st = LM_ST_TEXT);
	///
	~MathFuncInset();
	///
	MathedInset * Clone();
#ifdef USE_PAINTER
	///
	void draw(Painter &, int, int);
#else
	///
	void Draw(int, int);
#endif
	///
	void Write(ostream &);
	///
	void Write(string & file);
	///
	void Metrics();
	///
	inline bool GetLimits() const;
protected:
	///
	int ln;
	///
	bool lims;
	///
	char * fname;
};


/// Accents
class MathAccentInset: public MathedInset {
public:
	///
	MathAccentInset(byte, MathedTextCodes, int, short st = LM_ST_TEXT);
	///
	MathAccentInset(MathedInset *, int, short st = LM_ST_TEXT);
	///
	~MathAccentInset();
	///
	MathedInset * Clone();
#ifdef USE_PAINTER
	///
	void draw(Painter &, int, int);
#else
	///
	void Draw(int, int);
#endif
	///
	void Write(ostream &);
	///
	void Write(string & file);
	///
	void Metrics();
	///
	int getAccentCode() const { return code; }
	
protected:
	///
	byte c;
	///
	MathedTextCodes fn;
	///
	int code;
	///
	MathedInset * inset;
	///
	int dh, dy;
};


///
class MathDotsInset: public MathedInset {
public:
	///
	MathDotsInset(char const *, int, short st = LM_ST_TEXT);
	///
	MathedInset * Clone();
#ifdef USE_PAINTER
	///
	void draw(Painter &, int, int);
#else
	///
	void Draw(int, int);
#endif
	///
	void Write(ostream &);
	///
	void Write(string & file);
	///
	void Metrics();
protected:
	///
	int dh, code;
};   


/// Smart spaces
class MathSpaceInset: public MathedInset  {
public:
	///
	MathSpaceInset(int sp, short ot = LM_OT_SPACE, short st = LM_ST_TEXT);
	///
	MathedInset * Clone();
#ifdef USE_PAINTER
	///
	void draw(Painter &, int, int);
#else
	///
	void Draw(int, int);
#endif
	///
	void Write(ostream &);
	///
	void Write(string & file);
	///
	inline void Metrics();
	///
	inline void SetSpace(int sp);
	///
	int GetSpace() { return space; }
protected:
	///
	int space;
};


/// big operators
class MathBigopInset: public MathedInset {
public:
	///
	MathBigopInset(char const *, int, short st = LM_ST_TEXT);
	///
	MathedInset * Clone();
#ifdef USE_PAINTER
	///
	void draw(Painter &, int, int);
#else
	///
	void Draw(int, int);
#endif
	///
	void Write(ostream &);
	///
	void Write(string & file);
	///
	void Metrics();
	///
	inline bool GetLimits() const;
	///
	inline void SetLimits(bool);
protected:
	///
	int lims;
	///
	int sym;   
};

 
//------- All editable insets must be derived from MathParInset.

///
class MathSqrtInset: public MathParInset {
public:
	///
	MathSqrtInset(short st = LM_ST_TEXT);
	///
	MathedInset * Clone();
#ifdef USE_PAINTER
	void draw(Painter &, int x, int baseline);
#else
	///
	void Draw(int x, int baseline);
#endif
	///
	void Write(ostream &);
	///
	void Write(string & file);
	///
	void Metrics();
	///
	bool Inside(int, int);
private:
	///
	int hmax, wbody;
};


/// Fraction like objects (frac, stackrel, binom) 
class MathFracInset: public MathParInset {
public:
	///
	MathFracInset(short ot = LM_OT_FRAC);
	///
	~MathFracInset();
	///
	MathedInset * Clone();
#ifdef USE_PAINTER
	///
	void draw(Painter &, int x, int baseline);
#else
	///
	void Draw(int x, int baseline);
#endif
	///
	void Write(ostream &);
	///
	void Write(string & file);
	///
	void Metrics();
	
	/** This does the same that SetData(LyxArrayBase*) but for both
	    numerator and denominator at once.
	*/
	void SetData(LyxArrayBase *, LyxArrayBase *);
	///
	void SetData(LyxArrayBase *);
	///
	void GetXY(int & x, int & y) const;
	///
	void SetFocus(int, int);
	///
	bool Inside(int, int);
	///
	LyxArrayBase * GetData();
	///
	bool setArgumentIdx(int i); // was bool Up/down(void);
	///
	int  getArgumentIdx() { return int(idx); }
	///
	int  getMaxArgumentIdx() { return 1; }
	///
	void  SetStyle(short);
protected:
	///
	short idx;
	///
	MathParInset * den;
	///
	int w0, w1, des0, dh;
};


/// A delimiter
class MathDelimInset: public MathParInset {
public:
	///
	MathDelimInset(int, int, short st = LM_ST_TEXT);
	///
	MathedInset * Clone();
#ifdef USE_PAINTER
	///
	void draw(Painter &, int, int);
#else
	///
	void Draw(int, int);
#endif
	///
	void Write(ostream &);
	///
	void Write(string & file);
	///
	void Metrics();
protected:
	///
	int left, right;
	///
	int dw, dh;
};


/// Decorations over (below) a math object
class MathDecorationInset: public MathParInset {
public:
	///
	MathDecorationInset(int, short st = LM_ST_TEXT);
	///
	MathedInset * Clone();
#ifdef USE_PAINTER
	///
	void draw(Painter &, int, int);
#else
	///
	void Draw(int, int);
#endif
	///
	void Write(ostream &);
	///
	void Write(string & file);
	///
	void Metrics();
	///
	inline bool GetLimits() const;
protected:
	///
	int deco;
	///
	bool upper;
	///
	int dw, dh, dy;
};


// --------------------  Inline functions ---------------------

 
inline
MathFuncInset::~MathFuncInset()
{
    if (fname && GetType() == LM_OT_UNDEF) delete[] fname;
}


inline
bool MathFuncInset::GetLimits() const 
{  
   return bool(lims && (GetStyle() == LM_ST_DISPLAY)); 
} 


inline
void MathFuncInset::Write(ostream & os)
{
	os << "\\" << name;
}


inline
void MathFuncInset::Write(string & file)
{
   file += '\\';
   file += name;
   file += ' ';
}


inline
void MathSpaceInset::Metrics()
{
   width = space ? space * 2 : 2;
   if (space > 3) width *= 2;
   if (space == 5) width *= 2;
   width += 4;
   ascent = 4; descent = 0;
}


inline
void MathSpaceInset::SetSpace(int sp)
{ 
   space = sp;
   Metrics();
}    


inline
bool MathBigopInset::GetLimits() const 
{  
    // Default case
    if (lims < 0) {
	return sym != LM_int && sym != LM_oint && GetStyle() == LM_ST_DISPLAY;
    } 
    
    // Custom 
    return lims > 0;
} 


inline
void MathBigopInset::SetLimits(bool ls) 
{  
    lims = ls ? 1 : 0; 
} 


inline
bool MathDecorationInset::GetLimits() const
{ 
   return deco == LM_underbrace || deco == LM_overbrace;
}    

#endif
