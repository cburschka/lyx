// -*- C++ -*-
#ifndef LYX_CB_H
#define LYX_CB_H

#include FORMS_H_LOCATION

#include "LString.h"

class Inset;

///
extern bool quitting;

// When still false after reading lyxrc, warn user
//about failing \bind_file command. RVDK_PATCH_5
extern bool BindFileSet;

///
extern "C" void FootCB(FL_OBJECT *, long);
///
extern void EmphCB();
///
extern void BoldCB();
///
extern void NounCB();
///
extern "C" void MarginCB(FL_OBJECT *, long);
///
extern "C" void FigureCB(FL_OBJECT *, long);
///
extern "C" void TableCB(FL_OBJECT *, long);
///
extern "C" void MeltCB(FL_OBJECT *, long);
///
extern void TexCB();
///
extern "C" void DepthCB(FL_OBJECT *, long);
///
extern void FreeCB();
///
extern void HtmlUrlCB();
///
extern void UrlCB();
///
extern void ProhibitInput();
///
extern void CodeCB();
///
extern void SansCB();
///
extern void RomanCB();
///
extern void StyleResetCB();
///
extern void UnderlineCB();
///
extern void FontSizeCB(string const &);
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

