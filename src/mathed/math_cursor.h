// -*- C++ -*-
/*
 *  File:        math_cursor.h
 *  Purpose:     Declaration of interaction classes for mathed
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: MathCursor control all user interaction
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: (c) 1996, Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta, Mathed & Lyx project.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#ifndef MATH_CURSOR
#define MATH_CURSOR

#ifdef __GNUG__
#pragma interface
#endif

#include "math_iter.h" 
#include "math_inset.h"


/// This is the external interface of Mathed's subkernel
class MathedCursor {
 public:
    ///
    MathedCursor(MathParInset * p);
    ///
    ~MathedCursor() { };
    ///
    void Insert(byte, MathedTextCodes t = LM_TC_MIN);
    ///
    void Insert(MathedInset *, int t = LM_TC_INSET);
    ///
    void Home();
    ///
    void End();
    ///
    bool Right(bool sel = false);
    ///
    bool Left(bool sel = false);
    ///
    bool Up(bool sel = false);
    ///
    bool Down(bool sel = false);
    ///
    bool Pop();
    ///
    bool Push();
    /// Pull out an argument from its container inset
    bool pullArg();
#ifdef USE_PAINTER
    ///
    void draw(Painter &, int x, int y);
    ///
    void Redraw(Painter &);
#else
    ///
    void Draw(long unsigned pm, int x, int y);
    ///
    void Redraw();
#endif
    ///
    void Delete();
    ///
    void DelLine();
    ///
    void SetPos(int, int);
    ///
    void GetPos(int & x, int & y) { cursor->GetPos(x, y); }
    ///
    short GetFCode() { return cursor->FCode(); }
    ///
    MathParInset * GetPar() { return par; }
    ///
    MathParInset * getCurrentPar() const { return cursor->p; }
    ///
    void SetPar(MathParInset *);
    ///
    void Interpret(char const *);
    ///
    void SetSize(short);
    ///
    void setNumbered();
    void setLabel(char const *);
    ///
    bool Limits();
    /// Set accent: if argument = 0 it's considered consumed 
    void setAccent(int ac = 0);
    /// Returns last accent
    int getAccent() const;
    ///
    bool IsEnd() const { return !cursor->OK(); }
    // Macro mode methods
    ///
    void MacroModeOpen();
    ///
    void MacroModeClose();
    ///
    bool InMacroMode() { return macro_mode; }
    
    // Local selection methods
    ///
    bool Selection() { return selection; }
    ///
    void SelCopy();
    ///
    void SelCut();
    ///
    void SelDel();
    ///
    void SelPaste();
    ///
    void SelStart();
    ///
    void SelClear();
    ///
    void SelBalance();
#ifdef USE_PAINTER
	void SelGetArea(int ** xp, int ** yp, int & n);
#else
    ///
    XPoint * SelGetArea(int &);
#endif
    ///
    void clearLastCode() { lastcode = LM_TC_MIN; }
    ///
    void setLastCode(MathedTextCodes t) { lastcode = t; }
    ///
    MathedTextCodes getLastCode() const { return lastcode; }
    
 protected:
    ///
    bool macro_mode;
    ///
    void MacroModeBack();
    ///
    void MacroModeInsert(char);
    
    // Selection stuff
    ///
    bool selection;
    ///
    int  selpos;
    ///
    MathedXIter cursel, * anchor;
    ///
//    LyxArrayBase *selarray; 
    ///
    Bool is_visible;
    ///
    long unsigned win;
    ///
    MathParInset * par;
    ///
    MathedXIter * cursor;
    ///
    int xc, yc;
    ///
    void doAccent(byte c, MathedTextCodes t);
    ///
    void doAccent(MathedInset * p);
    ///
    int accent;
	///
    int nestaccent[8];
	///
    MathedTextCodes lastcode;

 private:
	///
    char macrobf[80];
	///
    int macroln;
	///
    MathFuncInset * imacro;
};


//--------------------   Inline Functions  -------------------------// 


#endif
