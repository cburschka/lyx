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
#include <string>

using std::endl;
using std::string;

namespace {

#ifdef WORDS_BIGENDIAN
	char const * ucs4_codeset = "UCS-4BE";
	char const * ucs2_codeset = "UCS-2BE";
#else
	char const * ucs4_codeset = "UCS-4LE";
	char const * ucs2_codeset = "UCS-2LE";
#endif

std::vector<char>
iconv_convert(std::string const & tocode, std::string const & fromcode,
	      std::vector<char> const & buf)
{
	if (buf.empty())
		return std::vector<char>();

	iconv_t cd = iconv_open(tocode.c_str(), fromcode.c_str());
	if (cd == (iconv_t)(-1)) {
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

	char ICONV_CONST * inbuf = const_cast<char ICONV_CONST *>(&buf[0]);
	size_t inbytesleft = buf.size();
	static char out[1000];
	char * outbuf = out;
	size_t outbytesleft = 1000;

	size_t res = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);

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
			for (size_t i = 0; i < buf.size(); ++i) {
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
			for (size_t i = 0; i < buf.size(); ++i) {
				unsigned char const b = buf[i];
				lyxerr << "0x" << int(b) << " ";
			}
			lyxerr << endl;
			break;
		default:
			lyxerr << "\tSome other error: " << errno << endl;
			break;
		}
	}

	if (iconv_close(cd) == -1) {
		lyxerr << "Error returned from iconv_close("
		       << errno << ")" << endl;
	}

	//lyxerr << std::dec;
	//lyxerr << "Inbytesleft: " << inbytesleft << endl;
	//lyxerr << "Outbytesleft: " << outbytesleft << endl;
	int bytes = 1000 - outbytesleft;

	std::vector<char> outvec(out, out + bytes);
	return outvec;
}


std::vector<boost::uint32_t> bytes_to_ucs4(std::vector<char> const & bytes)
{
	boost::uint32_t const * tmp = reinterpret_cast<uint32_t const *>(&bytes[0]);
	return std::vector<boost::uint32_t>(tmp, tmp + bytes.size() / 4);
}


std::vector<unsigned short> bytes_to_ucs2(std::vector<char> const & bytes)
{
	unsigned short const * tmp = reinterpret_cast<unsigned short const *>(&bytes[0]);
	return std::vector<unsigned short>(tmp, tmp + bytes.size() / 2);
}

} // anon namespace


std::vector<boost::uint32_t> utf8_to_ucs4(std::vector<char> const & utf8str)
{
	//lyxerr << "Buff = " << string(utf8str.begin(), utf8str.end())
	//       << " (" << utf8str.size() << ")" << endl;
	//lyxerr << "Res = " << string(res.begin(), res.end())
	//       << " (" << res.size() << ")" << endl;

	std::vector<char> res = iconv_convert(ucs4_codeset, "UTF-8", utf8str);
	return bytes_to_ucs4(res);
}


std::vector<boost::uint32_t>
ucs2_to_ucs4(std::vector<unsigned short> const & ucs2str)
{
	char const * tin = reinterpret_cast<char const *>(&ucs2str[0]);
	std::vector<char> in(tin, tin + ucs2str.size() * 2);
	std::vector<char> res = iconv_convert(ucs4_codeset, ucs2_codeset, in);
	return bytes_to_ucs4(res);
}


std::vector<unsigned short>
ucs4_to_ucs2(std::vector<boost::uint32_t> const & ucs4str)
{
	char const * tin = reinterpret_cast<char const *>(&ucs4str[0]);
	std::vector<char> in(tin, tin + ucs4str.size() * 4);
	std::vector<char> res = iconv_convert(ucs2_codeset, ucs4_codeset, in);
	return bytes_to_ucs2(res);
}


std::vector<unsigned short>
ucs4_to_ucs2(boost::uint32_t const * s, size_t ls)
{
	char const * tin = reinterpret_cast<char const *>(s);
	std::vector<char> in(tin, tin + ls * 4);
	std::vector<char> res = iconv_convert(ucs2_codeset, ucs4_codeset, in);
	return bytes_to_ucs2(res);
}


unsigned short
ucs4_to_ucs2(boost::uint32_t c)
{
	char const * tin = reinterpret_cast<char const *>(&c);
	std::vector<char> in(tin, tin + 4);
	std::vector<char> res = iconv_convert(ucs2_codeset, ucs4_codeset, in);
	return bytes_to_ucs2(res)[0];
}


std::vector<char> ucs4_to_utf8(std::vector<boost::uint32_t> const & ucs4str)
{
	char const * tin = reinterpret_cast<char const *>(&ucs4str[0]);
	std::vector<char> in(tin, tin + ucs4str.size() * 4);
	std::vector<char> res = iconv_convert("UTF-8", ucs4_codeset, in);
	return res;
}


std::vector<char> ucs4_to_utf8(boost::uint32_t c)
{
	char const * tin = reinterpret_cast<char const *>(&c);
	std::vector<char> in(tin, tin + 4);
	std::vector<char> res = iconv_convert("UTF-8", ucs4_codeset, in);
	return res;
}
