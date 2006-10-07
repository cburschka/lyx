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

using std::endl;

namespace {

#ifdef WORDS_BIGENDIAN
	char const * ucs4_codeset = "UCS-4BE";
	char const * ucs2_codeset = "UCS-2BE";
#else
	char const * ucs4_codeset = "UCS-4LE";
	char const * ucs2_codeset = "UCS-2LE";
#endif

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
	size_t const outsize = 1000;
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
				unsigned char const b = buf[i];
				lyxerr << "0x" << int(b) << " ";
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
				unsigned char const b = buf[i];
				lyxerr << "0x" << int(b) << " ";
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
	return ucs4_to_utf8(&ucs4str[0], ucs4str.size());
}


std::vector<char>
ucs4_to_utf8(lyx::char_type const * ucs4str, size_t ls)
{
	static iconv_t cd = (iconv_t)(-1);
	return iconv_convert<char>(&cd, "UTF-8", ucs4_codeset,
				   ucs4str, ls);
}
