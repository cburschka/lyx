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
bool MenuWrite(Buffer * buffer);
/// write the given file, or ask if no name given
bool WriteAs(Buffer * buffer, string const & filename = string());
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
string const getPossibleLabel(BufferView const & bv);
///
void Reconfigure(BufferView * bv);
#endif
