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
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
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

#include "LString.h"
#include "math_defs.h"
#include "symbol_def.h"

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
    ///
    static int workWidth;
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
#endif
