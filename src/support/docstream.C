/**
 * \file docstream.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "docstream.h"
#include "unicode.h"

#include <cerrno>
#include <cstdio>
#include <iconv.h>
#include <locale>

namespace {

char const * utf8_codeset = "UTF-8";

// We use C IO throughout this file, because the facets might be used with
// lyxerr in the future.


class utf8_codecvt_facet_exception : public std::exception {
public:
	virtual ~utf8_codecvt_facet_exception() throw() {}
	virtual const char* what() const throw()
	{
		return "iconv problem in utf8_codecvt_facet initialization";
	}
};


/// codecvt facet for conversion of UCS4 (internal representation) to UTF8
/// (external representation) or vice versa
class utf8_codecvt_facet : public std::codecvt<lyx::char_type, char, std::mbstate_t>
{
	typedef std::codecvt<lyx::char_type, char, std::mbstate_t> base;
public:
	/// Constructor. You have to specify with \p inout whether you want
	/// to use this facet only for input, only for output or for both.
	explicit utf8_codecvt_facet(std::ios_base::openmode inout = std::ios_base::in | std::ios_base::out,
			size_t refs = 0)
		: base(refs)
	{
		if (inout & std::ios_base::in) {
			in_cd_ = iconv_open(ucs4_codeset, utf8_codeset);
			if (in_cd_ == (iconv_t)(-1)) {
				fprintf(stderr, "Error %d returned from iconv_open(in_cd_): %s\n",
				        errno, strerror(errno));
				fflush(stderr);
				throw utf8_codecvt_facet_exception();
			}
		} else
			in_cd_ = (iconv_t)(-1);
		if (inout & std::ios_base::out) {
			out_cd_ = iconv_open(utf8_codeset, ucs4_codeset);
			if (out_cd_ == (iconv_t)(-1)) {
				fprintf(stderr, "Error %d returned from iconv_open(out_cd_): %s\n",
				        errno, strerror(errno));
				fflush(stderr);
				throw utf8_codecvt_facet_exception();
			}
		} else
			out_cd_ = (iconv_t)(-1);
	}
protected:
	virtual ~utf8_codecvt_facet()
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
		size_t inbytesleft = (from_end - from) * sizeof(intern_type);
		size_t outbytesleft = (to_end - to) * sizeof(extern_type);
		from_next = from;
		to_next = to;
		return do_iconv(out_cd_, reinterpret_cast<char const **>(&from_next),
				&inbytesleft, &to_next, &outbytesleft);
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
		return do_iconv(in_cd_, &from_next, &inbytesleft,
				reinterpret_cast<char **>(&to_next),
				&outbytesleft);
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
		return std::min(length, max);
#endif
	}
	virtual int do_max_length() const throw()
	{
		// UTF8 uses at most 6 bytes to represent one code point
		return 6;
	}
private:
	/// Do the actual conversion. The interface is equivalent to that of
	/// iconv() (but const correct).
	inline base::result do_iconv(iconv_t cd, char const ** from,
			size_t * inbytesleft, char ** to, size_t * outbytesleft) const
	{
		char const * to_start = *to;
		size_t converted = iconv(cd, const_cast<char ICONV_CONST **>(from),
				inbytesleft, to, outbytesleft);
		if (converted == (size_t)(-1)) {
			switch(errno) {
			case EINVAL:
			case E2BIG:
				return base::partial;
			case EILSEQ:
			default:
				fprintf(stderr, "Error %d returned from iconv: %s\n",
				        errno, strerror(errno));
				fflush(stderr);
				return base::error;
			}
		}
		if (*to == to_start)
			return base::noconv;
		return base::ok;
	}
	iconv_t in_cd_;
	iconv_t out_cd_;
};

}


namespace lyx {


idocfstream::idocfstream() : base()
{
	std::locale global;
	std::locale locale(global, new utf8_codecvt_facet(in));
	imbue(locale);
}

	
idocfstream::idocfstream(const char* s, std::ios_base::openmode mode)
	: base()
{
	// We must imbue the stream before openening the file
	std::locale global;
	std::locale locale(global, new utf8_codecvt_facet(in));
	imbue(locale);
	open(s, mode);
}


odocfstream::odocfstream() : base()
{
	std::locale global;
	std::locale locale(global, new utf8_codecvt_facet(out));
	imbue(locale);
}

	
odocfstream::odocfstream(const char* s, std::ios_base::openmode mode)
	: base()
{
	// We must imbue the stream before openening the file
	std::locale global;
	std::locale locale(global, new utf8_codecvt_facet(out));
	imbue(locale);
	open(s, mode);
}

}

#if (!defined(HAVE_WCHAR_T) || SIZEOF_WCHAR_T != 4) && defined(__GNUC__)
// We get undefined references to these virtual methods. This looks like
// a bug in gcc. The implementation here does not do anything useful, since
// it is overriden in utf8_codecvt_facet.
namespace std {
template<> codecvt<lyx::char_type, char, mbstate_t>::result
codecvt<lyx::char_type, char, mbstate_t>::do_out(mbstate_t &, const lyx::char_type *, const lyx::char_type *, const lyx::char_type *&,
		char *, char *, char *&) const { return error; }
template<> codecvt<lyx::char_type, char, mbstate_t>::result
codecvt<lyx::char_type, char, mbstate_t>::do_unshift(mbstate_t &, char *, char *, char *&) const { return error; }
template<> codecvt<lyx::char_type, char, mbstate_t>::result
codecvt<lyx::char_type, char, mbstate_t>::do_in(mbstate_t &, const char *, const char *, const char *&,
		lyx::char_type *, lyx::char_type *, lyx::char_type *&) const { return error; }
template<> int codecvt<lyx::char_type, char, mbstate_t>::do_encoding() const throw() { return 0; }
template<> bool codecvt<lyx::char_type, char, mbstate_t>::do_always_noconv() const throw() { return true; }
#if __GNUC__ == 3 && __GNUC_MINOR__ < 4
template<> int codecvt<lyx::char_type, char, mbstate_t>::do_length(mbstate_t const &, const char *, const char *, size_t) const { return 1; }
#else
template<> int codecvt<lyx::char_type, char, mbstate_t>::do_length(mbstate_t &, const char *, const char *, size_t) const { return 1; }
#endif
template<> int codecvt<lyx::char_type, char, mbstate_t>::do_max_length() const throw() { return 4; }
}
#endif
