/**
 * \file docstream.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/docstream.h"
#include "support/lstrings.h"
#include "support/unicode.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iconv.h>
#include <locale>

using namespace std;

using lyx::ucs4_codeset;


#if defined(_MSC_VER) && (_MSC_VER >= 1600)
std::locale::id numpunct<lyx::char_type>::id;
#endif


namespace {

// We use C IO throughout this file, because the facets might be used with
// lyxerr in the future.


/// codecvt facet for conversion of UCS4 (internal representation) to UTF8
/// (external representation) or vice versa
class iconv_codecvt_facet : public codecvt<lyx::char_type, char, mbstate_t>
{
	typedef codecvt<lyx::char_type, char, mbstate_t> base;
public:
	/// Constructor. You have to specify with \p inout whether you want
	/// to use this facet only for input, only for output or for both.
	explicit iconv_codecvt_facet(string const & encoding = "UTF-8",
			ios_base::openmode inout = ios_base::in | ios_base::out,
			size_t refs = 0)
		: base(refs), encoding_(encoding)
	{
		if (inout & ios_base::in) {
			in_cd_ = iconv_open(ucs4_codeset, encoding.c_str());
			if (in_cd_ == (iconv_t)(-1)) {
				fprintf(stderr, "Error %d returned from iconv_open(in_cd_): %s\n",
					errno, strerror(errno));
				fflush(stderr);
				throw lyx::iconv_codecvt_facet_exception();
			}
		} else
			in_cd_ = (iconv_t)(-1);
		if (inout & ios_base::out) {
			out_cd_ = iconv_open(encoding.c_str(), ucs4_codeset);
			if (out_cd_ == (iconv_t)(-1)) {
				fprintf(stderr, "Error %d returned from iconv_open(out_cd_): %s\n",
					errno, strerror(errno));
				fflush(stderr);
				throw lyx::iconv_codecvt_facet_exception();
			}
		} else
			out_cd_ = (iconv_t)(-1);
	}
protected:
	virtual ~iconv_codecvt_facet()
	{
		if (in_cd_ != (iconv_t)(-1))
			if (iconv_close(in_cd_) == -1) {
				fprintf(stderr, "Error %d returned from iconv_close(in_cd_): %s\n",
					errno, strerror(errno));
				fflush(stderr);
			}
		if (out_cd_ != (iconv_t)(-1))
			if (iconv_close(out_cd_) == -1) {
				fprintf(stderr, "Error %d returned from iconv_close(out_cd_): %s\n",
					errno, strerror(errno));
				fflush(stderr);
			}
	}
	virtual result do_out(state_type &, intern_type const * from,
			intern_type const * from_end, intern_type const *& from_next,
			extern_type * to, extern_type * to_end,
			extern_type *& to_next) const
	{
#define WORKAROUND_ICONV_BUG 1
#if WORKAROUND_ICONV_BUG
		// Due to a bug in some iconv versions, when the last char
		// in the buffer is a wide char and the output encoding is
		// ISO-2022-JP and we are going to switch to another encoding,
		// the appropriate escape sequence for changing the character
		// set is not output (see bugs 5216, 5280, and also 5489).
		// As a workaround, we append a nul char in order to force
		// a switch to ASCII, and then remove it from output after
		// the conversion.
		intern_type * from_new = 0;
		intern_type const * from_old = from;
		size_t extra = 0;
		if (*(from_end - 1) >= 0x80 && encoding_ == "ISO-2022-JP") {
			size_t len = from_end - from;
			from_new = new intern_type[len + 1];
			memcpy(from_new, from, len * sizeof(intern_type));
			from_new[len] = 0;
			from_end = from_new + len + 1;
			from = from_new;
			extra = 1;
		}
#endif
		size_t inbytesleft = (from_end - from) * sizeof(intern_type);
		size_t outbytesleft = (to_end - to) * sizeof(extern_type);
#if WORKAROUND_ICONV_BUG
		outbytesleft += extra * sizeof(extern_type);
#endif
		from_next = from;
		to_next = to;
		result const retval = do_iconv(out_cd_,
				reinterpret_cast<char const **>(&from_next),
				&inbytesleft, &to_next, &outbytesleft);
#if WORKAROUND_ICONV_BUG
		// Remove from output the nul char that we inserted at the end
		// of the input buffer in order to circumvent an iconv bug.
		if (from_new) {
			--to_next;
			--from_next;
			from_next = from_old + (from_next - from);
			from = from_old;
			delete[] from_new;
		}
#endif
		if (retval == base::error) {
			fprintf(stderr,
				"Error %d returned from iconv when converting from %s to %s: %s\n",
				errno, ucs4_codeset, encoding_.c_str(),
				strerror(errno));
			fputs("Converted input:", stderr);
			for (intern_type const * i = from; i < from_next; ++i) {
				unsigned int const c = *i;
				fprintf(stderr, " 0x%04x", c);
			}
			unsigned int const c = *from_next;
			fprintf(stderr, "\nStopped at: 0x%04x\n", c);
			fputs("Unconverted input:", stderr);
			for (intern_type const * i = from_next + 1; i < from_end; ++i) {
				unsigned int const c = *i;
				fprintf(stderr, " 0x%04x", c);
			}
			fputs("\nConverted output:", stderr);
			for (extern_type const * i = to; i < to_next; ++i) {
				// extern_type may be signed, avoid output of
				// something like 0xffffffc2
				unsigned int const c =
					*reinterpret_cast<unsigned char const *>(i);
				fprintf(stderr, " 0x%02x", c);
			}
			fputc('\n', stderr);
			fflush(stderr);
		}
		return retval;
	}
	virtual result do_unshift(state_type &, extern_type * to,
			extern_type *, extern_type *& to_next) const
	{
		// utf8 does not use shifting
		to_next = to;
		return base::noconv;
	}
	virtual result do_in(state_type &,
			extern_type const * from, extern_type const * from_end,
			extern_type const *& from_next,
			intern_type * to, intern_type * to_end,
			intern_type *& to_next) const
	{
		size_t inbytesleft = (from_end - from) * sizeof(extern_type);
		size_t outbytesleft = (to_end - to) * sizeof(intern_type);
		from_next = from;
		to_next = to;
		result const retval = do_iconv(in_cd_, &from_next, &inbytesleft,
				reinterpret_cast<char **>(&to_next),
				&outbytesleft);
		if (retval == base::error) {
			fprintf(stderr,
				"Error %d returned from iconv when converting from %s to %s: %s\n",
				errno, encoding_.c_str(), ucs4_codeset,
				strerror(errno));
			fputs("Converted input:", stderr);
			for (extern_type const * i = from; i < from_next; ++i) {
				// extern_type may be signed, avoid output of
				// something like 0xffffffc2
				unsigned int const c =
					*reinterpret_cast<unsigned char const *>(i);
				fprintf(stderr, " 0x%02x", c);
			}
			unsigned int const c =
				*reinterpret_cast<unsigned char const *>(from_next);
			fprintf(stderr, "\nStopped at: 0x%02x\n", c);
			fputs("Unconverted input:", stderr);
			for (extern_type const * i = from_next + 1; i < from_end; ++i) {
				unsigned int const c =
					*reinterpret_cast<unsigned char const *>(i);
				fprintf(stderr, " 0x%02x", c);
			}
			fputs("\nConverted output:", stderr);
			for (intern_type const * i = to; i < to_next; ++i) {
				unsigned int const c = *i;
				fprintf(stderr, " 0x%02x", c);
			}
			fputc('\n', stderr);
			fflush(stderr);
		}
		return retval;
	}
	virtual int do_encoding() const throw()
	{
		return 0;
	}
	virtual bool do_always_noconv() const throw()
	{
		return false;
	}
	virtual int do_length(state_type & /*state*/, extern_type const * from,
			extern_type const * end, size_t max) const
	{
		// The docs are a bit unclear about this method.
		// It seems that we should calculate the actual length of the
		// converted sequence, but that would not make sense, since
		// once could just do the conversion directly.
		// Therefore we just return the number of unconverted
		// characters, since that is the best guess we can do.
#if 0
		intern_type * to = new intern_type[max];
		intern_type * to_end = to + max;
		intern_type * to_next = to;
		extern_type const * from_next = from;
		do_in(state, from, end, from_next, to, to_end, to_next);
		delete[] to;
		return to_next - to;
#else
		size_t const length = end - from;
		return min(length, max);
#endif
	}
	virtual int do_max_length() const throw()
	{
		return lyx::max_encoded_bytes(encoding_);
	}
private:
	/// Do the actual conversion. The interface is equivalent to that of
	/// iconv() (but const correct).
	inline base::result do_iconv(iconv_t cd, char const ** from,
			size_t * inbytesleft, char ** to, size_t * outbytesleft) const
	{
		char const * const to_start = *to;
		size_t converted = iconv(cd, const_cast<char ICONV_CONST **>(from),
				inbytesleft, to, outbytesleft);
		if (converted == (size_t)(-1)) {
			switch(errno) {
			case 0: 
				// As strange as it may seem, this
				// does happen on windows when parsing
				// comments with accented chars in
				// tex2lyx. See the following thread
				// for details
				// http://thread.gmane.org/gmane.editors.lyx.devel/117636
				break;
			case EINVAL:
			case E2BIG:
				return base::partial;
			case EILSEQ:
			default:
				return base::error;
			}
		}
		if (*to == to_start)
			return base::noconv;
		return base::ok;
	}
	iconv_t in_cd_;
	iconv_t out_cd_;
	/// The narrow encoding
	string encoding_;
};

} // namespace anon


namespace lyx {

template<class Ios>
void setEncoding(Ios & ios, string const & encoding, ios_base::openmode mode)
{
	// We must imbue the stream before openening the file
	locale global;
	locale locale(global, new iconv_codecvt_facet(encoding, mode));
	ios.imbue(locale);
}


const char * iconv_codecvt_facet_exception::what() const throw()
{
	return "iconv problem in iconv_codecvt_facet initialization";
}


ifdocstream::ifdocstream() : base()
{
	setEncoding(*this, "UTF-8", in);
}


ifdocstream::ifdocstream(const char* s, ios_base::openmode mode,
			 string const & encoding)
	: base()
{
	setEncoding(*this, encoding, in);
	open(s, mode);
}


ofdocstream::ofdocstream(): base()
{
	setEncoding(*this, "UTF-8", out);
}


ofdocstream::ofdocstream(const char* s, ios_base::openmode mode,
			 string const & encoding)
	: base()
{
	setEncoding(*this, encoding, out);
	open(s, mode);
}


void ofdocstream::reset(string const & encoding)
{
	setEncoding(*this, encoding, out);
}



SetEnc setEncoding(string const & encoding)
{
	return SetEnc(encoding);
}


odocstream & operator<<(odocstream & os, SetEnc e)
{
	if (has_facet<iconv_codecvt_facet>(os.rdbuf()->getloc())) {
		// This stream must be a file stream, since we never imbue
		// any other stream with a locale having a iconv_codecvt_facet.
		// Flush the stream so that all pending output is written
		// with the old encoding.
		os.flush();
		locale locale(os.rdbuf()->getloc(),
			new iconv_codecvt_facet(e.encoding, ios_base::out));
		// FIXME Does changing the codecvt facet of an open file
		// stream always work? It does with gcc 4.1, but I have read
		// somewhere that it does not with MSVC.
		// What does the standard say?
		os.imbue(locale);
	}
	return os;
}


//CHECKME: I just copied the code above, and have no idea whether it
//is correct... (JMarc)
idocstream & operator<<(idocstream & is, SetEnc e)
{
	if (has_facet<iconv_codecvt_facet>(is.rdbuf()->getloc())) {
		// This stream must be a file stream, since we never imbue
		// any other stream with a locale having a iconv_codecvt_facet.
		// Flush the stream so that all pending output is written
		// with the old encoding.
		//is.flush();
		locale locale(is.rdbuf()->getloc(),
			new iconv_codecvt_facet(e.encoding, ios_base::in));
		// FIXME Does changing the codecvt facet of an open file
		// stream always work? It does with gcc 4.1, but I have read
		// somewhere that it does not with MSVC.
		// What does the standard say?
		is.imbue(locale);
	}
	return is;
}


#if ! defined(USE_WCHAR_T)
odocstream & operator<<(odocstream & os, char c)
{
	os.put(c);
	return os;
}
#endif

}


#if ! defined(USE_WCHAR_T) && defined(__GNUC__)
// We get undefined references to these virtual methods. This looks like
// a bug in gcc. The implementation here does not do anything useful, since
// it is overriden in iconv_codecvt_facet.
namespace std {

template<> codecvt<lyx::char_type, char, mbstate_t>::result
codecvt<lyx::char_type, char, mbstate_t>::do_out(
	mbstate_t &, const lyx::char_type *, const lyx::char_type *,
	const lyx::char_type *&, char *, char *, char *&) const
{
	return error;
}


template<> codecvt<lyx::char_type, char, mbstate_t>::result
codecvt<lyx::char_type, char, mbstate_t>::do_unshift(
	mbstate_t &, char *, char *, char *&) const
{
	return error;
}


template<> codecvt<lyx::char_type, char, mbstate_t>::result
codecvt<lyx::char_type, char, mbstate_t>::do_in(
	mbstate_t &, const char *, const char *, const char *&,
	lyx::char_type *, lyx::char_type *, lyx::char_type *&) const
{
	return error;
}


template<>
int codecvt<lyx::char_type, char, mbstate_t>::do_encoding() const throw()
{
	return 0;
}


template<>
bool codecvt<lyx::char_type, char, mbstate_t>::do_always_noconv() const throw()
{
	return true;
}

template<>
int codecvt<lyx::char_type, char, mbstate_t>::do_length(
	mbstate_t &, const char *, const char *, size_t) const
{
	return 1;
}

template<>
int codecvt<lyx::char_type, char, mbstate_t>::do_max_length() const throw()
{
	return 4;
}

} // namespace std
#endif
