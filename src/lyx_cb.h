// -*- C++ -*-
#ifndef _LYX_CB_H
#define _LYX_CB_H

#include FORMS_H_LOCATION

class LString;
class Inset;

///
extern bool quitting;

// When still false after reading lyxrc, warn user
//about failing \bind_file command. RVDK_PATCH_5
extern bool BindFileSet;

///
extern void FootCB(FL_OBJECT *, long);
///
extern void EmphCB();
///
extern void BoldCB();
///
extern void NounCB();
///
extern void MarginCB(FL_OBJECT *, long);
///
extern void FigureCB(FL_OBJECT *, long);
///
extern void TableCB(FL_OBJECT *, long);
///
extern void MeltCB(FL_OBJECT *, long);
///
extern void TexCB();
///
extern void DepthCB(FL_OBJECT *, long);
///
extern void FreeCB();
///
extern void CutCB();
///
extern void PasteCB();
///
extern void CopyCB();
///
extern void NoteCB();
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
extern void FontSizeCB(LString const &);
///
extern LString CurrentState();
///
extern void AllowInput();

///
struct InsetUpdateStruct {
	///
	Inset* inset;
	///
	InsetUpdateStruct* next;
};
#endif

