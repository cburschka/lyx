/**
 * \file docstring.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "docstring.h"
#include "unicode.h"

#include <boost/assert.hpp>


namespace lyx {

docstring const from_ascii(char const * ascii)
{
	docstring s;
	for (char const * c = ascii; *c; ++c) {
		BOOST_ASSERT(static_cast<unsigned char>(*c) < 0x80);
		s.push_back(*c);
	}
	return s;
}


docstring const from_ascii(std::string const & ascii)
{
	int const len = ascii.length();
	for (int i = 0; i < len; ++i)
		BOOST_ASSERT(static_cast<unsigned char>(ascii[i]) < 0x80);
	return docstring(ascii.begin(), ascii.end());
}


docstring const from_utf8(std::string const & utf8)
{
	std::vector<boost::uint32_t> const ucs4 =
		utf8_to_ucs4(std::vector<char>(utf8.begin(), utf8.end()));
	return docstring(ucs4.begin(), ucs4.end());
}


std::string const to_utf8(docstring const & ucs4)
{
	std::vector<char> const utf8 =
		ucs4_to_utf8(std::vector<boost::uint32_t>(ucs4.begin(), ucs4.end()));
	return std::string(utf8.begin(), utf8.end());
}

}


bool operator==(lyx::docstring const & l, char const * r)
{
	int const len = l.length();
	for (int i = 0; i < len; ++i) {
		BOOST_ASSERT(static_cast<unsigned char>(r[i]) < 0x80);
		if (!r[i])
			return false;
		if (l[i] != lyx::docstring::value_type(r[i]))
			return false;
	}
	return r[len] == '\0';
}
