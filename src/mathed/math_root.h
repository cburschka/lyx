// -*- C++ -*-
/*
 *  File:        math_root.h
 *  Purpose:     Declaration of the root object 
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1999
 *  Description: Root math object
 *
 *  Copyright: (c) 1999 Alejandro Aguilar Sierra
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#ifndef MATH_ROOT
#define MATH_ROOT

#ifdef __GNUG__
#pragma interface
#endif

#include <cstdio>
#include "math_defs.h"
#include "math_inset.h"
#include "symbol_def.h"
#include "LString.h"


///
class MathRootInset: public MathSqrtInset {
 public:
    ///
    MathRootInset(short st = LM_ST_TEXT);
    ///
    //    MathRootInset(MathSqrtInset &);
    ///
    ~MathRootInset();
    ///
    MathedInset * Clone();
    ///
    void Draw(int x, int baseline);
    ///
    void Write(ostream &);
    ///
    void Write(string & file);
    ///
    void Metrics();
    ///
    bool Inside(int, int);
    ///
    void SetFocus(int, int);
    ///
    void SetData(LyxArrayBase *);
    ///
    void GetXY(int& x, int& y) const;
    ///
    LyxArrayBase * GetData();
    ///
    bool setArgumentIdx(int i);
    ///
    int  getArgumentIdx() { return idx; }
    ///
    int  getMaxArgumentIdx() { return 1; }
    ///
    void  SetStyle(short);

 protected:
    ///
    int idx;
    ///
    MathParInset * uroot;
    ///
    int wroot, dh;
};
#endif
