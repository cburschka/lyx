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

#include <boost/cstdint.hpp>
#include <vector>

std::vector<boost::uint32_t>
utf8_to_ucs4(std::vector<char> const & utf8str);

std::vector<boost::uint32_t>
ucs2_to_ucs4(std::vector<unsigned short> const & ucs2str);

std::vector<unsigned short>
ucs4_to_ucs2(std::vector<boost::uint32_t> const & ucs4str);

std::vector<unsigned short>
ucs4_to_ucs2(boost::uint32_t const * s, size_t ls);

unsigned short
ucs4_to_ucs2(boost::uint32_t c);

std::vector<char>
ucs4_to_utf8(std::vector<boost::uint32_t> const & ucs4str);

std::vector<char>
ucs4_to_utf8(boost::uint32_t c);

#endif
