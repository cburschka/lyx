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
extern void Emph();
///
extern void Bold();
///
extern void Noun();
///
extern void Margin(BufferView *);
///
extern void Figure();
///
extern void RTLCB();
///
extern void Table();
///
extern void Melt(BufferView *);
///
extern void Tex();
///
extern void changeDepth(BufferView *, int);
///
extern void Free();
///
extern void HtmlUrl();
///
extern void Url();
///
extern void ProhibitInput();
///
extern void Code();
///
extern void Sans();
///
extern void Roman();
///
extern void StyleReset();
///
extern void Underline();
///
extern void FontSize(string const &);
///
extern string CurrentState();
///
extern void AllowInput();

///
struct InsetUpdateStruct {
	///
	Inset * inset;
	///
	InsetUpdateStruct * next;
};
#endif

