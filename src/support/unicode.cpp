/**
 * \file unicode.cpp
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

#include "support/unicode.h"
#include "support/debug.h"

#include <QThreadStorage>

#include <iconv.h>

#include <boost/cstdint.hpp>

#include <cerrno>
#include <map>
#include <ostream>
//Needed in MSVC
#include <string>


using namespace std;

namespace {

#ifdef WORDS_BIGENDIAN
	char const * utf16_codeset = "UTF16-BE";
#else
	char const * utf16_codeset = "UTF16-LE";
#endif

}


namespace lyx {

#ifdef WORDS_BIGENDIAN
	char const * ucs4_codeset = "UCS-4BE";
#else
	char const * ucs4_codeset = "UCS-4LE";
#endif

static const iconv_t invalid_cd = (iconv_t)(-1);


class IconvProcessor::Impl
{
public:
	// noncopyable because iconv_close() is called in destructor
	Impl(Impl const &) = delete;
	Impl & operator=(Impl const &) = delete;

	Impl(string const & to, string const & from)
		: cd(invalid_cd), tocode_(to), fromcode_(from)
	{}

	~Impl()
	{
		if (cd != invalid_cd && iconv_close(cd) == -1)
			LYXERR0("Error returned from iconv_close(" << errno << ')');
	}

	iconv_t cd;
	string tocode_;
	string fromcode_;
};


IconvProcessor::IconvProcessor(char const * tocode, char const * fromcode)
	: pimpl_(new IconvProcessor::Impl(tocode, fromcode))
{
}


IconvProcessor::IconvProcessor(IconvProcessor const & other)
	: pimpl_(new IconvProcessor::Impl(other.pimpl_->tocode_, other.pimpl_->fromcode_))
{
}


IconvProcessor::~IconvProcessor()
{
	delete pimpl_;
}


IconvProcessor & IconvProcessor::operator=(IconvProcessor const & other)
{
	if (&other != this) {
		delete pimpl_;
		pimpl_ = new Impl(other.pimpl_->tocode_, other.pimpl_->fromcode_);
	}
	return *this;
}


bool IconvProcessor::init()
{
	if (pimpl_->cd != invalid_cd)
		return true;

	pimpl_->cd = iconv_open(pimpl_->tocode_.c_str(), pimpl_->fromcode_.c_str());
	if (pimpl_->cd != invalid_cd)
		return true;

	lyxerr << "Error returned from iconv_open" << endl;
	switch (errno) {
		case EINVAL:
			lyxerr << "EINVAL The conversion from " << pimpl_->fromcode_
				<< " to " << pimpl_->tocode_
				<< " is not supported by the implementation."
				<< endl;
			break;
		default:
			lyxerr << "\tSome other error: " << errno << endl;
			break;
	}
	return false;
}


int IconvProcessor::convert(char const * buf, size_t buflen,
		char * outbuf, size_t maxoutsize)
{
	if (buflen == 0)
		return 0;

	if (pimpl_->cd == invalid_cd) {
		if (!init())
			return -1;
	}

	char ICONV_CONST * inbuf = const_cast<char ICONV_CONST *>(buf);
	size_t inbytesleft = buflen;
	size_t outbytesleft = maxoutsize;

	int res = iconv(pimpl_->cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);

	// flush out remaining data. This is needed because iconv sometimes
	// holds back chars in the stream, waiting for a combination character
	// (see e.g. http://sources.redhat.com/bugzilla/show_bug.cgi?id=1124)
	iconv(pimpl_->cd, NULL, NULL, &outbuf, &outbytesleft);

	//lyxerr << dec;
	//lyxerr << "Inbytesleft: " << inbytesleft << endl;
	//lyxerr << "Outbytesleft: " << outbytesleft << endl;

	if (res != -1)
		// Everything went well.
		return maxoutsize - outbytesleft;

	// There are some errors in the conversion
	lyxerr << "Error returned from iconv" << endl;
	switch (errno) {
		case E2BIG:
			lyxerr << "E2BIG  There is not sufficient room at *outbuf." << endl;
			break;
		case EILSEQ:
			lyxerr << "EILSEQ An invalid multibyte sequence"
				<< " has been encountered in the input.\n"
				<< "When converting from " << pimpl_->fromcode_
				<< " to " << pimpl_->tocode_ << ".\n";
			lyxerr << "Input:" << hex;
			for (size_t i = 0; i < buflen; ++i) {
				// char may be signed, avoid output of
				// something like 0xffffffc2
				boost::uint32_t const b =
					*reinterpret_cast<unsigned char const *>(buf + i);
				lyxerr << " 0x" << (unsigned int)b;
			}
			lyxerr << dec << endl;
			break;
		case EINVAL:
			lyxerr << "EINVAL An incomplete multibyte sequence"
				<< " has been encountered in the input.\n"
				<< "When converting from " << pimpl_->fromcode_
				<< " to " << pimpl_->tocode_ << ".\n";
			lyxerr << "Input:" << hex;
			for (size_t i = 0; i < buflen; ++i) {
				// char may be signed, avoid output of
				// something like 0xffffffc2
				boost::uint32_t const b =
					*reinterpret_cast<unsigned char const *>(buf + i);
				lyxerr << " 0x" << (unsigned int)b;
			}
			lyxerr << dec << endl;
			break;
		default:
			lyxerr << "\tSome other error: " << errno << endl;
			break;
	}
	// We got an error so we close down the conversion engine
	if (iconv_close(pimpl_->cd) == -1) {
		lyxerr << "Error returned from iconv_close("
			<< errno << ")" << endl;
	}
	pimpl_->cd = invalid_cd;
	return -1;
}


std::string IconvProcessor::from() const
{
	return pimpl_->fromcode_;
}


std::string IconvProcessor::to() const
{
	return pimpl_->tocode_;
}


namespace {


template<typename RetType, typename InType>
vector<RetType>
iconv_convert(IconvProcessor & processor, InType const * buf, size_t buflen)
{
	if (buflen == 0)
		return vector<RetType>();

	char const * inbuf = reinterpret_cast<char const *>(buf);
	size_t inbytesleft = buflen * sizeof(InType);

	static QThreadStorage<std::vector<char> *> static_outbuf;
	if (!static_outbuf.hasLocalData())
		static_outbuf.setLocalData(new std::vector<char>(32768));
	std::vector<char> & outbuf = *static_outbuf.localData();
	// The number of UCS4 code points in buf is at most inbytesleft.
	// The output encoding will use at most
	// max_encoded_bytes(pimpl_->tocode_) per UCS4 code point.
	size_t maxoutbufsize = max_encoded_bytes(processor.to()) * inbytesleft;
	if (outbuf.size() < maxoutbufsize)
		outbuf.resize(maxoutbufsize);

	int bytes = processor.convert(inbuf, inbytesleft, &outbuf[0], outbuf.size());
	if (bytes <= 0)
		// Conversion failed
		// FIXME Maybe throw an exception and handle that in the caller?
		return vector<RetType>();

	RetType const * tmp = reinterpret_cast<RetType const *>(&outbuf[0]);
	return vector<RetType>(tmp, tmp + bytes / sizeof(RetType));
}

} // anon namespace


IconvProcessor & utf8ToUcs4()
{
	static QThreadStorage<IconvProcessor *> processor;
	if (!processor.hasLocalData())
		processor.setLocalData(new IconvProcessor(ucs4_codeset, "UTF-8"));
	return *processor.localData();
}


vector<char_type> utf8_to_ucs4(vector<char> const & utf8str)
{
	if (utf8str.empty())
		return vector<char_type>();

	return utf8_to_ucs4(&utf8str[0], utf8str.size());
}


vector<char_type>
utf8_to_ucs4(char const * utf8str, size_t ls)
{
	return iconv_convert<char_type>(utf8ToUcs4(), utf8str, ls);
}


vector<char_type>
utf16_to_ucs4(unsigned short const * s, size_t ls)
{
	static QThreadStorage<IconvProcessor *> processor;
	if (!processor.hasLocalData())
		processor.setLocalData(new IconvProcessor(ucs4_codeset, utf16_codeset));
	return iconv_convert<char_type>(*processor.localData(), s, ls);
}


vector<unsigned short>
ucs4_to_utf16(char_type const * s, size_t ls)
{
	static QThreadStorage<IconvProcessor *> processor;
	if (!processor.hasLocalData())
		processor.setLocalData(new IconvProcessor(utf16_codeset, ucs4_codeset));
	return iconv_convert<unsigned short>(*processor.localData(), s, ls);
}


IconvProcessor & ucs4ToUtf8()
{
	static QThreadStorage<IconvProcessor *> processor;
	if (!processor.hasLocalData())
		processor.setLocalData(new IconvProcessor("UTF-8", ucs4_codeset));
	return *processor.localData();
}


vector<char>
ucs4_to_utf8(char_type c)
{
	return iconv_convert<char>(ucs4ToUtf8(), &c, 1);
}


vector<char>
ucs4_to_utf8(vector<char_type> const & ucs4str)
{
	if (ucs4str.empty())
		return vector<char>();

	return ucs4_to_utf8(&ucs4str[0], ucs4str.size());
}


vector<char>
ucs4_to_utf8(char_type const * ucs4str, size_t ls)
{
	return iconv_convert<char>(ucs4ToUtf8(), ucs4str, ls);
}


vector<char_type>
eightbit_to_ucs4(char const * s, size_t ls, string const & encoding)
{
	static QThreadStorage<map<string, IconvProcessor> *> static_processors;
	if (!static_processors.hasLocalData())
		static_processors.setLocalData(new map<string, IconvProcessor>);
	map<string, IconvProcessor> & processors = *static_processors.localData();
	if (processors.find(encoding) == processors.end()) {
		IconvProcessor processor(ucs4_codeset, encoding.c_str());
		processors.insert(make_pair(encoding, processor));
	}
	return iconv_convert<char_type>(processors[encoding], s, ls);
}


namespace {

map<string, IconvProcessor> & ucs4To8bitProcessors()
{
	static QThreadStorage<map<string, IconvProcessor> *> processors;
	if (!processors.hasLocalData())
		processors.setLocalData(new map<string, IconvProcessor>);
	return *processors.localData();
}

}


vector<char>
ucs4_to_eightbit(char_type const * ucs4str, size_t ls, string const & encoding)
{
	map<string, IconvProcessor> & processors(ucs4To8bitProcessors());
	if (processors.find(encoding) == processors.end()) {
		IconvProcessor processor(encoding.c_str(), ucs4_codeset);
		processors.insert(make_pair(encoding, processor));
	}
	return iconv_convert<char>(processors[encoding], ucs4str, ls);
}


char ucs4_to_eightbit(char_type ucs4, string const & encoding)
{
	map<string, IconvProcessor> & processors(ucs4To8bitProcessors());
	map<string, IconvProcessor>::iterator it = processors.find(encoding);
	if (it == processors.end()) {
		IconvProcessor processor(encoding.c_str(), ucs4_codeset);
		it = processors.insert(make_pair(encoding, processor)).first;
	}

	char out;
	int const bytes = it->second.convert((char *)(&ucs4), 4, &out, 1);
	if (bytes > 0)
		return out;
	return 0;
}


void ucs4_to_multibytes(char_type ucs4, vector<char> & out,
	string const & encoding)
{
	static QThreadStorage<map<string, IconvProcessor> *> static_processors;
	if (!static_processors.hasLocalData())
		static_processors.setLocalData(new map<string, IconvProcessor>);
	map<string, IconvProcessor> & processors = *static_processors.localData();
	map<string, IconvProcessor>::iterator it = processors.find(encoding);
	if (it == processors.end()) {
		IconvProcessor processor(encoding.c_str(), ucs4_codeset);
		it = processors.insert(make_pair(encoding, processor)).first;
	}

	out.resize(4);
	int bytes = it->second.convert((char *)(&ucs4), 4, &out[0], 4);
	if (bytes > 0)
		out.resize(bytes);
	else
		out.clear();
}

int max_encoded_bytes(std::string const & encoding)
{
	// FIXME: this information should be transferred to lib/encodings
	// UTF8 uses at most 4 bytes to represent one UCS4 code point
	// (see RFC 3629). RFC 2279 specifies 6 bytes, but that
	// information is outdated, and RFC 2279 has been superseded by
	// RFC 3629.
	// The CJK encodings use (different) multibyte representation as well.
	// All other encodings encode one UCS4 code point in one byte
	// (and can therefore only encode a subset of UCS4)
	// Furthermore, all encodings that use shifting (like SJIS) do not work with
	// iconv_codecvt_facet.
	if (encoding == "UTF-8" ||
	    encoding == "GB" ||
	    encoding == "EUC-TW")
		return 4;
	else if (encoding == "EUC-JP")
		return 3;
	else if (encoding == "ISO-2022-JP")
		return 8;
	else if (encoding == "BIG5" ||
	         encoding == "EUC-KR" ||
	         encoding == "EUC-CN" ||
	         encoding == "SJIS" ||
	         encoding == "GBK")
		return 2;
	else
		return 1;
}

} // namespace lyx
