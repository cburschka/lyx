// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef LYX_STY_H
#define LYX_STY_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

// This seems to be fixed now (JMarc)
//#ifdef WITH_WARNINGS
//#warning README!!
//#endif
// I suddenly began to get segfaults in guilsinglleft_def and had no clue
// on why this happened, then I tried a rewrite and added lyx_sty.C. Now
// the segfault has moved to bibcombox... It seems that we(or gcc) have a
// problem with static (non-pointer) objects , if someone knows why please
// tell me. I am using gcc 2.7.2.1 (Linux) and when I get this segfault it
// always has to do with a static object of some kind... I tried to rewrite
// the objects causing the segfault, but the segfault then just moves to
// another object. I'll let some others have a look now... (Lgb)

// include this always
///
extern string const lyx_def;
///
extern string const lyxline_def;
///
extern string const noun_def;
///
extern string const lyxarrow_def;
///
extern string const quotedblbase_def;
///
extern string const quotesinglbase_def;
///
extern string const guillemotleft_def;
///
extern string const guillemotright_def;
///
extern string const guilsinglleft_def;
///
extern string const guilsinglright_def;
///
extern string const paragraphleftindent_def;
///
extern string const floatingfootnote_def;
///
extern string const minipageindent_def;
///
extern string const boldsymbol_def;
///
extern string const binom_def;
///
extern string const mathcircumflex_def;

#endif
