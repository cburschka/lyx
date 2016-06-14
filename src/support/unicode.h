// -*- C++ -*-
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
#include "support/unique_ptr.h"

#include <cstddef>
#include <string>
#include <vector>


namespace lyx {

/**
 * Wrapper for iconv(3).
 *
 * According to the POSIX standard, all specified functions are thread-safe,
 * with some exceptions. The iconv() function is not listed as an exception:
 * http://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xsh_chap02.html#tag_22_02_09_09
 * http://man7.org/linux/man-pages/man7/pthreads.7.html
 *
 * Therefore, you can use as many instances of this class in parallel as you
 * like. However, you need to ensure that each instance is only used by one
 * thread at any given time. If this condition is not met you get nasty
 * mixtures of different thread data as in bug 7240.
 *
 * From a performance point of view it is best to use one static instance
 * per thread for each in/out encoding pair. This can e.g. be achieved by
 * using helpers for thread-local storage such as QThreadStorage or
 * boost::thread_specific_ptr. A single static instance protected by a mutex
 * would work as well, and might be preferrable for exotic encoding pairs.
 * Creating local IconvProcessor instances should be avoided because of the
 * overhead in iconv_open().
 */
class IconvProcessor
{
	/// open iconv.
	/// \return true if the processor is ready to use.
	bool init();
	std::string const tocode_;
	std::string const fromcode_;
	struct Handler;
	unique_ptr<Handler> h_;
public:
	IconvProcessor(std::string tocode, std::string fromcode);
	/// convert any data from \c fromcode to \c tocode unicode format.
	/// \return the number of bytes of the converted output buffer.
	int convert(char const * in_buffer, size_t in_size,
		char * out_buffer, size_t max_out_size);
	/// target encoding
	std::string to() const { return tocode_; }
	// required by g++ 4.6
	IconvProcessor(IconvProcessor && other);
};

/// Get the global IconvProcessor instance of the current thread for
/// utf8->ucs4 conversions
IconvProcessor & utf8ToUcs4();

// A single codepoint conversion for utf8_to_ucs4 does not make
// sense, so that function is left out.

std::vector<char_type> utf8_to_ucs4(std::vector<char> const & utf8str);

std::vector<char_type> utf8_to_ucs4(char const * utf8str, size_t ls);

// utf16_to_ucs4

std::vector<char_type> utf16_to_ucs4(unsigned short const * s, size_t ls);

// ucs4_to_utf16

std::vector<unsigned short> ucs4_to_utf16(char_type const * s, size_t ls);

/// Get the global IconvProcessor instance of the current thread for
/// ucs4->utf8 conversions
IconvProcessor & ucs4ToUtf8();

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

/// How many bytes does one UCS4 code point use at most in encoding \p encoding?
int max_encoded_bytes(std::string const & encoding);

} // namespace lyx

#endif
