// -*- C++ -*-
#ifndef LYX_CB_H
#define LYX_CB_H

#include "LString.h"

class Buffer;
class BufferView;

///
extern bool quitting;

///
void ShowMessage(Buffer const * buf,
		 string const & msg1,
		 string const & msg2 = string(),
		 string const & msg3 = string());
///
bool MenuWrite(BufferView * bv, Buffer * buffer);
/// write the given file, or ask if no name given
bool WriteAs(BufferView * bv, Buffer * buffer,
	     string const & filename = string());
///
int MenuRunChktex(Buffer * buffer);
///
void QuitLyX();
///
void AutoSave(BufferView * bv);
///
Buffer * NewFile(string const & filename);
///
void InsertAsciiFile(BufferView * bv, string const & f, bool asParagraph);
///
string getContentsOfAsciiFile(BufferView * bv, string const & f, bool asParagraph);
///
void MenuInsertLabel(BufferView * bv, string const & arg);
///
void Reconfigure(BufferView * bv);
#endif
