/**
 * \file unicode.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 *
 * A collection of unicode conversion functions, using iconv.
 */

#ifndef LYX_SUPPORT_UNICODE_H
#define LYX_SUPPORT_UNICODE_H

#include "support/types.h"

#include <vector>

// utf8_to_ucs4

// A single codepoint conversion for utf8_to_ucs4 does not make
// sense, so that function is left out.

std::vector<lyx::char_type>
utf8_to_ucs4(std::vector<char> const & utf8str);

std::vector<lyx::char_type>
utf8_to_ucs4(char const * utf8str, size_t ls);

// ucs2_to_ucs4

lyx::char_type
ucs2_to_ucs4(unsigned short c);

std::vector<lyx::char_type>
ucs2_to_ucs4(std::vector<unsigned short> const & ucs2str);

std::vector<lyx::char_type>
ucs2_to_ucs4(unsigned short const * ucs2str, size_t ls);

// ucs4_to_ucs2

unsigned short
ucs4_to_ucs2(lyx::char_type c);

std::vector<unsigned short>
ucs4_to_ucs2(std::vector<lyx::char_type> const & ucs4str);

std::vector<unsigned short>
ucs4_to_ucs2(lyx::char_type const * s, size_t ls);

// ucs4_to_utf8

std::vector<char>
ucs4_to_utf8(lyx::char_type c);

std::vector<char>
ucs4_to_utf8(std::vector<lyx::char_type> const & ucs4str);

std::vector<char>
ucs4_to_utf8(lyx::char_type const * ucs4str, size_t ls);

extern char const * ucs4_codeset;
extern char const * ucs2_codeset;

#endif
