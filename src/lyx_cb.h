// -*- C++ -*-
#ifndef LYX_CB_H
#define LYX_CB_H

#include FORMS_H_LOCATION

#include "LString.h"

class Inset;
class BufferView;

///
extern bool quitting;

// When still false after reading lyxrc, warn user
//about failing \bind_file command. RVDK_PATCH_5
extern bool BindFileSet;

///
extern void Foot(BufferView *);
///
extern void Emph(BufferView *);
///
extern void Bold(BufferView *);
///
extern void Noun(BufferView *);
///
extern void Margin(BufferView *);
///
extern void Figure();
///
extern void Lang(BufferView *, string const &);
///
extern void Table();
///
extern void Melt(BufferView *);
///
extern void Tex(BufferView *);
///
extern void changeDepth(BufferView *, int);
///
extern void Free(BufferView *);
///
extern void ProhibitInput(BufferView *);
///
extern void AllowInput(BufferView *);
///
extern void Code(BufferView *);
///
extern void Sans(BufferView *);
///
extern void Roman(BufferView *);
///
extern void StyleReset(BufferView *);
///
extern void Underline(BufferView *);
///
extern void FontSize(BufferView *, string const &);
///
extern string CurrentState(BufferView *);

#endif

