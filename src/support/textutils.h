// -*- C++ -*-
/**
 * \file textutils.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Matthias Ettrich
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

// FIXME: I can think of a better name for this file ...

#ifndef TEXTUTILS_H
#define TEXTUTILS_H

#include "support/strfwd.h"


namespace lyx {

/// return true if the char is a line separator
inline bool isLineSeparatorChar(char_type c) { return c == ' '; }

/// return true if a char is alphabetical (including accented chars)
bool isLetterChar(char_type c);

/// return true if a char is lowercase
bool isLower(char_type c);

/// return whether \p c is an alphabetic character in the ASCII range
bool isAlphaASCII(char_type c);

/// return true if the char is printable
bool isPrintable(char_type c);

/// return true if the char is printable and not a space
bool isPrintableNonspace(char_type c);

/// return true if a unicode char is a space.
bool isSpace(char_type c);

/// return true if a unicode char is a digit.
bool isDigit(char_type c);

/// return whether \p c is a digit in the ASCII range
bool isDigitASCII(char_type c);

} // namespace lyx

#endif // TEXTUTILS_H
