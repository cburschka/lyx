// -*- C++ -*-
#ifndef BUFFERVIEW_FUNCS_H
#define BUFFERVIEW_FUNCS_H

#include "LString.h"

class BufferView;
class LyXFont;

#ifndef NEW_INSETS
///
extern void Foot(BufferView *);
#endif
///
extern void Emph(BufferView *);
///
extern void Bold(BufferView *);
///
extern void Noun(BufferView *);
#ifndef NEW_INSETS
///
extern void Margin(BufferView *);
#endif
///
extern void Figure();
///
extern void Table();
///
extern void Lang(BufferView *, string const &);
#ifndef NEW_INSETS
///
extern void Melt(BufferView *);
#endif
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
/// Returns the current font and depth as a message. 
extern string CurrentState(BufferView *);
///
extern void ToggleAndShow(BufferView *, LyXFont const &);
#endif
