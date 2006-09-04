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
	//lyxerr << "Outbuf =" << std::hex;

	std::vector<boost::uint32_t> ucs4;
	for (size_t i = 0; i < bytes.size(); i += 4) {
		unsigned char const b1 = bytes[i    ];
		unsigned char const b2 = bytes[i + 1];
		unsigned char const b3 = bytes[i + 2];
		unsigned char const b4 = bytes[i + 3];

		boost::uint32_t c;
		char * cc = reinterpret_cast<char *>(&c);
#ifdef WORDS_BIGENDIAN
		cc[0] = b1;
		cc[1] = b2;
		cc[2] = b3;
		cc[3] = b4;
#else
		cc[3] = b1;
		cc[2] = b2;
		cc[1] = b3;
		cc[0] = b4;
#endif

		if (c > 0xffff) {
			lyxerr << "Strange ucs4 value encountered\n";
			lyxerr << "0x"
			       << std::setw(2) << std::setfill('0') << int(b1)
			       << std::setw(2) << std::setfill('0') << int(b2)
			       << std::setw(2) << std::setfill('0') << int(b3)
			       << std::setw(2) << std::setfill('0') << int(b4)
			       << ' '
			       << "(0x"
			       << c
			       << ") ";
		}

		ucs4.push_back(c);
	}
	//lyxerr << endl;
	return ucs4;
}


std::vector<unsigned short> bytes_to_ucs2(std::vector<char> const & bytes)
{
	//lyxerr << "Outbuf =" << std::hex;

	std::vector<unsigned short> ucs2;
	for (size_t i = 0; i < bytes.size(); i += 2) {
		unsigned char const b1 = bytes[i    ];
		unsigned char const b2 = bytes[i + 1];

		unsigned short c;
		char * cc = reinterpret_cast<char *>(&c);
#ifdef WORDS_BIGENDIAN
		cc[0] = b1;
		cc[1] = b2;
#else
		cc[1] = b1;
		cc[0] = b2;
#endif

		//lyxerr << "0x"
		//       << std::setw(2) << std::setfill('0') << int(b2)
		//       << std::setw(2) << std::setfill('0') << int(b1)
		//       << ' '
		//       << "(0x"
		//       << c
		//       << ") ";

		ucs2.push_back(c);
	}
	//lyxerr << endl;
	return ucs2;
}

} // anon namespace


std::vector<boost::uint32_t> utf8_to_ucs4(std::vector<char> const & utf8str)
{
	//lyxerr << "Buff = " << string(utf8str.begin(), utf8str.end())
	//       << " (" << utf8str.size() << ")" << endl;
	//lyxerr << "Res = " << string(res.begin(), res.end())
	//       << " (" << res.size() << ")" << endl;

	std::vector<char> res = iconv_convert("UCS-4BE", "UTF-8", utf8str);
	return bytes_to_ucs4(res);
}


std::vector<boost::uint32_t>
ucs2_to_ucs4(std::vector<unsigned short> const & ucs2str)
{
	// TODO: Simplify and speed up.
	std::vector<char> in;
	std::vector<unsigned short>::const_iterator cit = ucs2str.begin();
	std::vector<unsigned short>::const_iterator end = ucs2str.end();
	//lyxerr << std::hex;
	for (; cit != end; ++cit) {
		unsigned short s = *cit;
		in.push_back(static_cast<char>((s & 0xff00) >> 8));
		in.push_back(static_cast<char>(s & 0x00ff));
		lyxerr << std::setw(2) << std::setfill('0') << ((s & 0xff00) >> 8) << endl;
		lyxerr << std::setw(2) << std::setfill('0') << (s & 0x00ff) << endl;
	}

	std::vector<char> res = iconv_convert("UCS-4BE", "UCS-2BE", in);
	return bytes_to_ucs4(res);
}


std::vector<unsigned short>
ucs4_to_ucs2(std::vector<boost::uint32_t> const & ucs4str)
{
	std::vector<char> in;
	std::vector<boost::uint32_t>::const_iterator cit = ucs4str.begin();
	std::vector<boost::uint32_t>::const_iterator end = ucs4str.end();
	for (; cit != end; ++cit) {
		boost::uint32_t s = *cit;
		in.push_back(static_cast<char>((s & 0xff000000) >> 24));
		in.push_back(static_cast<char>((s & 0x00ff0000) >> 16));
		in.push_back(static_cast<char>((s & 0x0000ff00) >> 8));
		in.push_back(static_cast<char>(s & 0x000000ff));
	}
	std::vector<char> res = iconv_convert("UCS-2BE", "UCS-4BE", in);
	return bytes_to_ucs2(res);
}


std::vector<unsigned short>
ucs4_to_ucs2(boost::uint32_t const * s, size_t ls)
{
	std::vector<char> in;
	for (size_t i = 0; i < ls; ++i) {
		in.push_back(static_cast<char>((s[i] & 0xff000000) >> 24));
		in.push_back(static_cast<char>((s[i] & 0x00ff0000) >> 16));
		in.push_back(static_cast<char>((s[i] & 0x0000ff00) >> 8));
		in.push_back(static_cast<char>(s[i] & 0x000000ff));
	}
	std::vector<char> res = iconv_convert("UCS-2BE", "UCS-4BE", in);
	return bytes_to_ucs2(res);
}


unsigned short
ucs4_to_ucs2(boost::uint32_t c)
{
	std::vector<char> in;
	in.push_back(static_cast<char>((c & 0xff000000) >> 24));
	in.push_back(static_cast<char>((c & 0x00ff0000) >> 16));
	in.push_back(static_cast<char>((c & 0x0000ff00) >> 8));
	in.push_back(static_cast<char>(c & 0x000000ff));
	std::vector<char> res = iconv_convert("UCS-2BE", "UCS-4BE", in);
	std::vector<unsigned short> us = bytes_to_ucs2(res);
	if (!us.empty())
		return us[0];
	else
		return 0xfffd; // unknown character
}


std::vector<char> ucs4_to_utf8(std::vector<boost::uint32_t> const & ucs4str)
{
	std::vector<char> in;
	std::vector<boost::uint32_t>::const_iterator cit = ucs4str.begin();
	std::vector<boost::uint32_t>::const_iterator end = ucs4str.end();
	for (; cit != end; ++cit) {
		boost::uint32_t s = *cit;
		in.push_back(static_cast<char>((s & 0xff000000) >> 24));
		in.push_back(static_cast<char>((s & 0x00ff0000) >> 16));
		in.push_back(static_cast<char>((s & 0x0000ff00) >> 8));
		in.push_back(static_cast<char>(s & 0x000000ff));
	}
	std::vector<char> res = iconv_convert("UTF-8", "UCS-4BE", in);
	return res;
}


std::vector<char> ucs4_to_utf8(boost::uint32_t c)
{
	std::vector<char> in;
	in.push_back(static_cast<char>((c & 0xff000000) >> 24));
	in.push_back(static_cast<char>((c & 0x00ff0000) >> 16));
	in.push_back(static_cast<char>((c & 0x0000ff00) >> 8));
	in.push_back(static_cast<char>(c & 0x000000ff));
	std::vector<char> res = iconv_convert("UTF-8", "UCS-4BE", in);
	return res;
}
