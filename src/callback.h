// -*- C++ -*-
/**
 * \file callback.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CALLBACK_H
#define CALLBACK_H

#include "support/docstring.h"

namespace lyx {

class Buffer;
class BufferView;

namespace frontend {
class LyXView;
}

///
extern bool quitting;

///
bool menuWrite(Buffer * buffer);
/// write the given file, or ask if no name given
bool writeAs(Buffer * buffer, std::string const & filename = std::string());
///
void autoSave(BufferView * bv);
///
void newFile(frontend::LyXView & lv, std::string const & filename);
///
void insertPlaintextFile(BufferView * bv, std::string const & f, bool asParagraph);
/// read plain text file (if \p f is empty, prompt for a filename)
docstring const getContentsOfPlaintextFile(BufferView * bv,
		std::string const & f, bool asParagraph);
///
void reconfigure(frontend::LyXView & lv, std::string const & option);

} // namespace lyx

#endif
