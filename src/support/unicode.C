/**
 * \file unicode.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 *
 * A collection of unicode conversion functions, using iconv.
 */

#include <config.h>

#include "unicode.h"

#include "debug.h"

#include <iconv.h>

#include <cerrno>
#include <iomanip>
#include <map>


namespace lyx {

using std::endl;

#ifdef WORDS_BIGENDIAN
	char const * ucs4_codeset = "UCS-4BE";
	char const * ucs2_codeset = "UCS-2BE";
#else
	char const * ucs4_codeset = "UCS-4LE";
	char const * ucs2_codeset = "UCS-2LE";
#endif

namespace {

template<typename RetType, typename InType>
std::vector<RetType>
iconv_convert(iconv_t * cd,
	      char const * tocode,
	      char const * fromcode,
	      InType const * buf,
	      size_t buflen)
{
	if (buflen == 0)
		return std::vector<RetType>();

	if (*cd == (iconv_t)(-1)) {
		*cd = iconv_open(tocode, fromcode);
		if (*cd == (iconv_t)(-1)) {
			lyxerr << "Error returned from iconv_open" << endl;
			switch (errno) {
			case EINVAL:
				lyxerr << "EINVAL The conversion from " << fromcode
				       << " to " << tocode
				       << " is not supported by the implementation."
				       << endl;
				break;
			default:
				lyxerr << "\tSome other error: " << errno << endl;
				break;
			}
		}
	}

	char ICONV_CONST * inbuf = const_cast<char ICONV_CONST *>(reinterpret_cast<char const *>(buf));
	size_t inbytesleft = buflen * sizeof(InType);
	// The preamble of the user guide is more than 11.500 characters, so we go for 32kb
	size_t const outsize = 32768;
	static char out[outsize];
	char * outbuf = out;
	size_t outbytesleft = outsize;

	size_t res = iconv(*cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);

	if (res == (size_t)(-1)) {
		lyxerr << "Error returned from iconv" << endl;
		switch (errno) {
		case E2BIG:
			lyxerr << "E2BIG  There is not sufficient room at *outbuf." << endl;
			break;
		case EILSEQ:
			lyxerr << "EILSEQ An invalid multibyte sequence"
			       << " has been encountered in the input.\n"
			       << "When converting from " << fromcode
			       << " to " << tocode << ".\n";
			lyxerr << "Input: " << std::hex;
			for (size_t i = 0; i < buflen; ++i) {
				boost::uint32_t const b = buf[i];
				lyxerr << "0x" << b << " ";
			}
			lyxerr << endl;
			break;
		case EINVAL:
			lyxerr << "EINVAL An incomplete multibyte sequence"
			       << " has been encountered in the input.\n"
			       << "When converting from " << fromcode
			       << " to " << tocode << ".\n";
			lyxerr << "Input: " << std::hex;
			for (size_t i = 0; i < buflen; ++i) {
				boost::uint32_t const b = buf[i];
				lyxerr << "0x" << b << " ";
			}
			lyxerr << endl;
			break;
		default:
			lyxerr << "\tSome other error: " << errno << endl;
			break;
		}
		// We got an error so we close down the conversion engine
		if (iconv_close(*cd) == -1) {
			lyxerr << "Error returned from iconv_close("
			       << errno << ")" << endl;
		}
		*cd = (iconv_t)(-1);
	}

	//lyxerr << std::dec;
	//lyxerr << "Inbytesleft: " << inbytesleft << endl;
	//lyxerr << "Outbytesleft: " << outbytesleft << endl;
	int bytes = outsize - outbytesleft;

	RetType const * tmp = reinterpret_cast<RetType const *>(out);
	return std::vector<RetType>(tmp, tmp + bytes / sizeof(RetType));
}

} // anon namespace


std::vector<lyx::char_type> utf8_to_ucs4(std::vector<char> const & utf8str)
{
	if (utf8str.empty())
		return std::vector<lyx::char_type>();

	return utf8_to_ucs4(&utf8str[0], utf8str.size());
}


std::vector<lyx::char_type>
utf8_to_ucs4(char const * utf8str, size_t ls)
{
	static iconv_t cd = (iconv_t)(-1);
	return iconv_convert<lyx::char_type>(&cd, ucs4_codeset, "UTF-8",
					      utf8str, ls);
}


lyx::char_type
ucs2_to_ucs4(unsigned short c)
{
	return ucs2_to_ucs4(&c, 1)[0];
}


std::vector<lyx::char_type>
ucs2_to_ucs4(std::vector<unsigned short> const & ucs2str)
{
	if (ucs2str.empty())
		return std::vector<lyx::char_type>();

	return ucs2_to_ucs4(&ucs2str[0], ucs2str.size());
}


std::vector<lyx::char_type>
ucs2_to_ucs4(unsigned short const * ucs2str, size_t ls)
{
	static iconv_t cd = (iconv_t)(-1);
	return iconv_convert<lyx::char_type>(&cd, ucs4_codeset, ucs2_codeset,
					      ucs2str, ls);
}


unsigned short
ucs4_to_ucs2(lyx::char_type c)
{
	return ucs4_to_ucs2(&c, 1)[0];
}


std::vector<unsigned short>
ucs4_to_ucs2(std::vector<lyx::char_type> const & ucs4str)
{
	if (ucs4str.empty())
		return std::vector<unsigned short>();

	return ucs4_to_ucs2(&ucs4str[0], ucs4str.size());
}


std::vector<unsigned short>
ucs4_to_ucs2(lyx::char_type const * s, size_t ls)
{
	static iconv_t cd = (iconv_t)(-1);
	return iconv_convert<unsigned short>(&cd, ucs2_codeset, ucs4_codeset,
					     s, ls);
}


std::vector<char>
ucs4_to_utf8(lyx::char_type c)
{
	static iconv_t cd = (iconv_t)(-1);
	return iconv_convert<char>(&cd, "UTF-8", ucs4_codeset, &c, 1);
}


std::vector<char>
ucs4_to_utf8(std::vector<lyx::char_type> const & ucs4str)
{
	if (ucs4str.empty())
		return std::vector<char>();

	return ucs4_to_utf8(&ucs4str[0], ucs4str.size());
}


std::vector<char>
ucs4_to_utf8(lyx::char_type const * ucs4str, size_t ls)
{
	static iconv_t cd = (iconv_t)(-1);
	return iconv_convert<char>(&cd, "UTF-8", ucs4_codeset,
				   ucs4str, ls);
}


std::vector<lyx::char_type>
eightbit_to_ucs4(char const * s, size_t ls, std::string const & encoding)
{
	static std::map<std::string, iconv_t> cd;
	if (cd.find(encoding) == cd.end())
		cd[encoding] = (iconv_t)(-1);
	return iconv_convert<char_type>(&cd[encoding], ucs4_codeset,
	                                encoding.c_str(), s, ls);
}


std::vector<char>
ucs4_to_eightbit(lyx::char_type const * ucs4str, size_t ls, std::string const & encoding)
{
	static std::map<std::string, iconv_t> cd;
	if (cd.find(encoding) == cd.end())
		cd[encoding] = (iconv_t)(-1);
	return iconv_convert<char>(&cd[encoding], encoding.c_str(),
	                           ucs4_codeset, ucs4str, ls);
}

} // namespace lyx
