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
bool menuWrite(Buffer * buffer);
/// write the given file, or ask if no name given
bool writeAs(Buffer * buffer, std::string const & filename = std::string());
///
void autoSave(BufferView * bv);
///
void newFile(BufferView * bv, std::string const & filename);
///
void insertAsciiFile(BufferView * bv, std::string const & f, bool asParagraph);
///
std::string getContentsOfAsciiFile(BufferView * bv, std::string const & f, bool asParagraph);
///
void reconfigure(BufferView * bv);
#endif
