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
 *  Copyright: 1996, Alejandro Aguilar Sierra
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

#include "math_xiter.h"

class MathFuncInset;
class MathParInset;
class Painter;


/// This is the external interface of Mathed's subkernel
class MathedCursor {
public:
	///
	explicit
	MathedCursor(MathParInset * p);
	///
	void Insert(byte, MathedTextCodes t = LM_TC_MIN);
	///
	void insertInset(MathedInset *, int t);
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
	///
	void draw(Painter &, int x, int y);
	///
	void Redraw(Painter &);
	///
	void Delete();
	///
	void DelLine();
	///
	void SetPos(int, int);
	///
	void GetPos(int & x, int & y);
	///
	short GetFCode();
	///
	MathParInset * GetPar();
	///
	MathParInset * getCurrentPar() const;
	///
	void SetPar(MathParInset *);
	///
	void Interpret(string const &);
	///
	void SetSize(short);
	///
	void setNumbered();
	///
	void setLabel(string const &);
	///
	string const & getLabel() const;
	///
	bool Limits();
	/// Set accent: if argument = 0 it's considered consumed 
	void setAccent(int ac = 0);
	/// Returns last accent
	int getAccent() const;
	///
	bool IsEnd() const;
	// Macro mode methods
	///
	void MacroModeOpen();
	///
	void MacroModeClose();
	///
	bool InMacroMode();
	
	// Local selection methods
	///
	bool Selection();
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
	///
	void SelGetArea(int ** xp, int ** yp, int & n);
	///
	void clearLastCode();
	///
	void setLastCode(MathedTextCodes t);
	///
	void toggleLastCode(MathedTextCodes t);
	///
	MathedTextCodes getLastCode() const;
	
protected:
	///
	bool macro_mode;
	
	// Selection stuff
	///
	bool selection;
	///
	int  selpos;
	///
	MathedXIter cursel;
	///
	MathedXIter * anchor;
	///
	MathParInset * par;
	///
	MathedXIter * cursor;
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
	MathFuncInset * imacro;
};

#endif
