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

#ifndef BUFFERVIEW_FUNCS_H
#define BUFFERVIEW_FUNCS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

class BufferView;
class LyXFont;
class LyXText;

///
extern void emph(BufferView *);
///
extern void bold(BufferView *);
///
extern void noun(BufferView *);
///
extern void lang(BufferView *, string const &);
///
extern void number(BufferView *);
///
extern void tex(BufferView *);
///
extern void changeDepth(BufferView *, LyXText *, int);
///
extern void code(BufferView *);
///
extern void sans(BufferView *);
///
extern void roman(BufferView *);
///
extern void styleReset(BufferView *);
///
extern void underline(BufferView *);
///
extern void fontSize(BufferView *, string const &);
/// Returns the current font and depth as a message.
extern string const currentState(BufferView *);
///
extern void toggleAndShow(BufferView *, LyXFont const &,
			  bool toggleall = true);
#endif
