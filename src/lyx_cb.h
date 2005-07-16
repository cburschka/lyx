// -*- C++ -*-
/**
 * \file lyx_cb.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_CB_H
#define LYX_CB_H

#include <string>

class Buffer;
class BufferView;

///
extern bool quitting;

///
bool MenuWrite(Buffer * buffer);
/// write the given file, or ask if no name given
bool WriteAs(Buffer * buffer, std::string const & filename = std::string());
///
void QuitLyX(bool noask);
///
void AutoSave(BufferView * bv);
///
void NewFile(BufferView * bv, std::string const & filename);
///
void InsertAsciiFile(BufferView * bv, std::string const & f, bool asParagraph);
///
std::string getContentsOfAsciiFile(BufferView * bv, std::string const & f, bool asParagraph);
///
void Reconfigure(BufferView * bv);
#endif
