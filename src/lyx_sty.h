// -*- C++ -*-
/*
  This file is part of
  ======================================================
  
  LyX, The Document Processor
  
  Copyright (C) 1995 Matthias Ettrich
  
  ======================================================
  */

#ifndef _LYX_STY_H
#define _LYX_STY_H

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
extern LString const lyx_def;
extern LString const lyxline_def;
extern LString const noun_def;
extern LString const lyxarrow_def;
extern LString const quotedblbase_def;
extern LString const quotesinglbase_def;
extern LString const guillemotleft_def;
extern LString const guillemotright_def;
extern LString const guilsinglleft_def;
extern LString const guilsinglright_def;
extern LString const paragraphindent_def;
extern LString const floatingfootnote_def;
extern LString const minipageindent_def;
extern LString const boldsymbol_def;
extern LString const binom_def;

#endif
