// -*- C++ -*-
#ifndef BUFFERVIEW_FUNCS_H
#define BUFFERVIEW_FUNCS_H

#include "LString.h"

class BufferView;
class LyXFont;

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
extern void Table();
///
extern void Lang(BufferView *, string const &);
///
extern void Number(BufferView *);
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
///
extern void ToggleAndShow(BufferView *, LyXFont const &);
#endif
