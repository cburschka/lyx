// -*- C++ -*-
#ifndef LYX_CB_H
#define LYX_CB_H

#include "LString.h"
#include "lyxfont.h"

class BufferParams;
class BufferView;
class Combox;

///
extern bool quitting;
///
extern bool toggleall;

// When still false after reading lyxrc, warn user
//about failing \bind_file command. RVDK_PATCH_5
///
extern bool BindFileSet;
///
void ShowMessage(Buffer const * buf,
		 string const & msg1,
		 string const & msg2 = string(),
		 string const & msg3 = string(), int delay = 6);
///
bool MenuWrite(BufferView * bv, Buffer * buffer);
///
bool MenuWriteAs(BufferView * bv, Buffer * buffer);
///
int MenuRunChktex(Buffer * buffer);
///
void QuitLyX();
///
void AutoSave(BufferView * bv);
///
Buffer * NewLyxFile(string const & filename);
///
void InsertAsciiFile(BufferView * bv, string const & f, bool asParagraph);
///
void MenuInsertLabel(BufferView * bv, string const & arg);
///
void LayoutsCB(int sel, void *, Combox *);
///
void MenuLayoutCharacter();
///
void MenuLayoutSave(BufferView * bv);
///
void Figure();
///
void Reconfigure(BufferView * bv);

	
#endif

