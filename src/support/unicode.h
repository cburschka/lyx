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

#include <boost/scoped_ptr.hpp>

#include <string>
#include <vector>


namespace lyx {

class IconvProcessor
{
public:
	IconvProcessor(
		char const * tocode = "",
		char const * fromcode = "");
	/// copy constructor needed because of pimpl_
	IconvProcessor(IconvProcessor const &);
	/// assignment operator needed because of pimpl_
	IconvProcessor & operator=(IconvProcessor const &);
	/// destructor (needs to be implemented in the .C file because the
	/// boost::scoped_ptr destructor needs a fully defined type
	~IconvProcessor();

	/// convert any data from \c fromcode to \c tocode unicode format.
	/// \return the number of bytes of the converted output buffer.
	int convert(
		char const * in_buffer,
		size_t in_size,
		char * out_buffer,
		size_t max_out_size);
private:
	/// open iconv.
	/// \return true if the processor is ready to use.
	bool init();

	std::string tocode_;
	std::string fromcode_;

	struct Private;
	boost::scoped_ptr<Private> pimpl_;
};

/// This is implemented in lyx_main.C for the LyX program 
/// and in client.C for the LyX client program.
extern IconvProcessor & utf8ToUcs4();

// A single codepoint conversion for utf8_to_ucs4 does not make
// sense, so that function is left out.

std::vector<lyx::char_type> utf8_to_ucs4(std::vector<char> const & utf8str);

std::vector<lyx::char_type> utf8_to_ucs4(char const * utf8str, size_t ls);

// ucs2_to_ucs4

lyx::char_type ucs2_to_ucs4(unsigned short c);

std::vector<lyx::char_type>
ucs2_to_ucs4(std::vector<unsigned short> const & ucs2str);

std::vector<lyx::char_type>
ucs2_to_ucs4(unsigned short const * ucs2str, size_t ls);

// ucs4_to_ucs2

unsigned short ucs4_to_ucs2(lyx::char_type c);

std::vector<unsigned short>
ucs4_to_ucs2(std::vector<lyx::char_type> const & ucs4str);

std::vector<unsigned short> ucs4_to_ucs2(lyx::char_type const * s, size_t ls);

// ucs4_to_utf8

std::vector<char> ucs4_to_utf8(lyx::char_type c);

std::vector<char> ucs4_to_utf8(std::vector<lyx::char_type> const & ucs4str);

std::vector<char> ucs4_to_utf8(lyx::char_type const * ucs4str, size_t ls);

/// convert \p s from encoding \p encoding to ucs4.
/// \p encoding must be a valid iconv 8bit encoding
std::vector<lyx::char_type>
eightbit_to_ucs4(char const * s, size_t ls, std::string const & encoding);

/// convert \p s from ucs4 to encoding \p encoding.
/// \p encoding must be a valid iconv 8bit encoding
std::vector<char>
ucs4_to_eightbit(lyx::char_type const * ucs4str, size_t ls, std::string const & encoding);

extern char const * ucs4_codeset;
extern char const * ucs2_codeset;


} // namespace lyx

#endif
