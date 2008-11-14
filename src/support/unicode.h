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

#include "support/strfwd.h"

#include <vector>


namespace lyx {

class IconvProcessor
{
public:
	IconvProcessor(char const * tocode = "", char const * fromcode = "");
	/// copy constructor needed because of pimpl_
	IconvProcessor(IconvProcessor const &);
	/// assignment operator needed because of pimpl_
	void operator=(IconvProcessor const &);
	/// destructor
	~IconvProcessor();

	/// convert any data from \c fromcode to \c tocode unicode format.
	/// \return the number of bytes of the converted output buffer.
	int convert(char const * in_buffer, size_t in_size,
		char * out_buffer, size_t max_out_size);

private:
	/// open iconv.
	/// \return true if the processor is ready to use.
	bool init();
	/// hide internals
	struct Impl;
	Impl * pimpl_;
};

// A single codepoint conversion for utf8_to_ucs4 does not make
// sense, so that function is left out.

std::vector<char_type> utf8_to_ucs4(std::vector<char> const & utf8str);

std::vector<char_type> utf8_to_ucs4(char const * utf8str, size_t ls);

// utf16_to_ucs4

std::vector<char_type> utf16_to_ucs4(unsigned short const * s, size_t ls);

// ucs4_to_utf16

std::vector<unsigned short> ucs4_to_utf16(char_type const * s, size_t ls);

// ucs4_to_utf8

std::vector<char> ucs4_to_utf8(char_type c);

std::vector<char> ucs4_to_utf8(std::vector<char_type> const & ucs4str);

std::vector<char> ucs4_to_utf8(char_type const * ucs4str, size_t ls);

/// convert \p s from encoding \p encoding to ucs4.
/// \p encoding must be a valid iconv 8bit encoding
std::vector<char_type>
eightbit_to_ucs4(char const * s, size_t ls, std::string const & encoding);

/// convert \p s from ucs4 to encoding \p encoding.
/// \p encoding must be a valid iconv 8bit encoding
std::vector<char> ucs4_to_eightbit(char_type const * ucs4str,
	size_t ls, std::string const & encoding);

/// convert ucs4 character \p c to encoding \p encoding.
/// \p encoding must be a valid iconv 8bit encoding
char ucs4_to_eightbit(char_type c, std::string const & encoding);

///
void ucs4_to_multibytes(char_type ucs4, std::vector<char> & out,
	std::string const & encoding);

extern char const * ucs4_codeset;


} // namespace lyx

#endif
