/**
 * \file docstring.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/docstring.h"

#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/qstring_helpers.h"
#include "support/unicode.h"

#include <QFile>

//Needed in Ubuntu
#include <typeinfo>
#if ! defined(USE_WCHAR_T) && defined(__GNUC__)
#include <locale>
#include <iostream>
#endif

using namespace std;

using lyx::support::isHexChar;

namespace lyx {

docstring const from_ascii(char const * ascii)
{
	docstring s;
	if (int n = strlen(ascii)) {
		s.resize(n);
		char_type *d = &s[0];
		while (--n >= 0) {
			d[n] = ascii[n];
			LATTEST(static_cast<unsigned char>(ascii[n]) < 0x80);
		}
	}
	return s;
}


docstring const from_ascii(string const & ascii)
{
	int const len = ascii.length();
	for (int i = 0; i < len; ++i)
		LATTEST(static_cast<unsigned char>(ascii[i]) < 0x80);
	return docstring(ascii.begin(), ascii.end());
}


string const to_ascii(docstring const & ucs4)
{
	int const len = ucs4.length();
	string ascii;
	ascii.resize(len);
	for (int i = 0; i < len; ++i) {
		LATTEST(ucs4[i] < 0x80);
		ascii[i] = static_cast<char>(ucs4[i]);
	}
	return ascii;
}


void utf8_to_ucs4(string const & utf8, docstring & ucs4)
{
	size_t n = utf8.size();
	// as utf8 is a multi-byte encoding, there would be at most
	// n characters:
	ucs4.resize(n);
	if (n == 0)
		return;

	int maxoutsize = n * 4;
	// basic_string::data() is not recognized by some old gcc version
	// so we use &(ucs4[0]) instead.
	char * outbuf = (char *)(&(ucs4[0]));
	int bytes = utf8ToUcs4().convert(utf8.c_str(), n, outbuf, maxoutsize);

	// adjust to the real converted size
	ucs4.resize(bytes/4);
}


docstring const from_utf8(string const & utf8)
{
	docstring ucs4;
	utf8_to_ucs4(utf8, ucs4);
	return ucs4;
}


string const to_utf8(docstring const & ucs4)
{
	vector<char> const utf8 = ucs4_to_utf8(ucs4.data(), ucs4.size());
	return string(utf8.begin(), utf8.end());
}


docstring const from_local8bit(string const & s)
{
	return qstring_to_ucs4(QString::fromLocal8Bit(s.data(), s.length()));
}


/// Exception thrown by to_local8bit if the string could not be converted
class to_local8bit_failure : public bad_cast {
public:
	to_local8bit_failure() throw() : bad_cast() {}
	virtual ~to_local8bit_failure() throw() {}
	virtual const char* what() const throw()
	{
		return "A string could not be converted from unicode to the local 8 bit encoding.";
	}
};


string const to_local8bit(docstring const & s)
{
	// This conversion can fail, depending on input.
	if (s.empty())
		return string();
	QByteArray const local = toqstr(s).toLocal8Bit();
	if (local.isEmpty())
		throw to_local8bit_failure();
	return string(local.begin(), local.end());
}


docstring const from_filesystem8bit(string const & s)
{
	QByteArray const encoded(s.c_str(), s.length());
	return qstring_to_ucs4(QFile::decodeName(encoded));
}


string const to_filesystem8bit(docstring const & s)
{
	QByteArray const encoded = QFile::encodeName(toqstr(s));
	return string(encoded.begin(), encoded.end());
}


string const to_iconv_encoding(docstring const & s, string const & encoding)
{
	std::vector<char> const encoded =
		ucs4_to_eightbit(s.data(), s.length(), encoding);
	return string(encoded.begin(), encoded.end());
}


docstring const from_iconv_encoding(string const & s, string const & encoding)
{
	std::vector<char_type> const ucs4 =
		eightbit_to_ucs4(s.data(), s.length(), encoding);
	return docstring(ucs4.begin(), ucs4.end());
}


docstring const normalize_c(docstring const & s)
{
	return qstring_to_ucs4(toqstr(s).normalized(QString::NormalizationForm_C));
}


bool operator==(lyx::docstring const & l, char const * r)
{
	lyx::docstring::const_iterator it = l.begin();
	lyx::docstring::const_iterator end = l.end();
	for (; it != end; ++it, ++r) {
		LASSERT(static_cast<unsigned char>(*r) < 0x80, return false);
		if (!*r)
			return false;
		if (*it != static_cast<lyx::docstring::value_type>(*r))
			return false;
	}
	return *r == '\0';
}


lyx::docstring operator+(lyx::docstring const & l, char const * r)
{
	lyx::docstring s(l);
	for (char const * c = r; *c; ++c) {
		LASSERT(static_cast<unsigned char>(*c) < 0x80, return l);
		s.push_back(*c);
	}
	return s;
}


lyx::docstring operator+(char const * l, lyx::docstring const & r)
{
	lyx::docstring s;
	for (char const * c = l; *c; ++c) {
		LASSERT(static_cast<unsigned char>(*c) < 0x80, return r);
		s.push_back(*c);
	}
	s += r;
	return s;
}


lyx::docstring operator+(lyx::docstring const & l, char r)
{
	LASSERT(static_cast<unsigned char>(r) < 0x80, return l);
	docstring s = l;
	s += docstring::value_type(r);
	return s;
}


lyx::docstring operator+(char l, lyx::docstring const & r)
{
	LASSERT(static_cast<unsigned char>(l) < 0x80, return r);
	return lyx::docstring::value_type(l) + r;
}


lyx::docstring & operator+=(lyx::docstring & l, char const * r)
{
	for (char const * c = r; *c; ++c) {
		LASSERT(static_cast<unsigned char>(*c) < 0x80, return l);
		l.push_back(*c);
	}
	return l;
}


lyx::docstring & operator+=(lyx::docstring & l, char r)
{
	LASSERT(static_cast<unsigned char>(r) < 0x80, return l);
	l.push_back(r);
	return l;
}

} // namespace lyx

#if ! defined(USE_WCHAR_T) && defined(__GNUC__)

// gcc does not have proper locale facets for lyx::char_type if
// sizeof(wchar_t) == 2, so we have to implement them on our own.


// We get undefined references to these virtual methods. This looks like
// a bug in gcc. The implementation here does not do anything useful, since
// it is overriden in ascii_ctype_facet.
namespace std {
template<> ctype<lyx::char_type>::~ctype() {}
template<> bool
ctype<lyx::char_type>::do_is(ctype<lyx::char_type>::mask, lyx::char_type) const { return false; }
template<> lyx::char_type const *
ctype<lyx::char_type>::do_is(const lyx::char_type *, const lyx::char_type *, ctype<lyx::char_type>::mask *) const { return 0; }
template<> const lyx::char_type *
ctype<lyx::char_type>::do_scan_is(ctype<lyx::char_type>::mask, const lyx::char_type *, const lyx::char_type *) const { return 0; }
template<> const lyx::char_type *
ctype<lyx::char_type>::do_scan_not(ctype<lyx::char_type>::mask, const lyx::char_type *, const lyx::char_type *) const { return 0; }
template<> lyx::char_type ctype<lyx::char_type>::do_toupper(lyx::char_type) const { return 0; }
template<> const lyx::char_type * ctype<lyx::char_type>::do_toupper(lyx::char_type *, lyx::char_type const *) const { return 0; }
template<> lyx::char_type ctype<lyx::char_type>::do_tolower(lyx::char_type) const { return 0; }
template<> const lyx::char_type * ctype<lyx::char_type>::do_tolower(lyx::char_type *, lyx::char_type const *) const { return 0; }
template<> lyx::char_type ctype<lyx::char_type>::do_widen(char) const { return 0; }
template<> const char *
ctype<lyx::char_type>::do_widen(const char *, const char *, lyx::char_type *) const { return 0; }
template<> char
ctype<lyx::char_type>::do_narrow(const lyx::char_type, char) const { return 0; }
template<> const lyx::char_type *
ctype<lyx::char_type>::do_narrow(const lyx::char_type *, const lyx::char_type *, char, char *) const { return 0; }
}


namespace lyx {

class ctype_failure : public bad_cast {
public:
	ctype_failure() throw() : bad_cast() {}
	virtual ~ctype_failure() throw() {}
	virtual const char* what() const throw()
	{
		return "The ctype<lyx::char_type> locale facet does only support ASCII characters on this platform.";
	}
};


class num_put_failure : public bad_cast {
public:
	num_put_failure() throw() : bad_cast() {}
	virtual ~num_put_failure() throw() {}
	virtual const char* what() const throw()
	{
		return "The num_put locale facet does only support ASCII characters on this platform.";
	}
};


/// ctype facet for UCS4 characters. The implementation does only support pure
/// ASCII, since we do not need anything else for now.
/// The code is partly stolen from ctype<wchar_t> from gcc.
class ascii_ctype_facet : public ctype<lyx::char_type>
{
public:
	typedef lyx::char_type char_type;
	typedef wctype_t wmask_type;
	explicit ascii_ctype_facet(size_t refs = 0) : ctype<char_type>(refs)
	{
		M_initialize_ctype();
	}
protected:
	bool       M_narrow_ok;
	char       M_narrow[128];
	wint_t     M_widen[1 + static_cast<unsigned char>(-1)];
	mask       M_bit[16];
	wmask_type M_wmask[16];
	wmask_type M_convert_to_wmask(const mask m) const
	{
		wmask_type ret;
		switch (m) {
			case space:  ret = wctype("space");  break;
			case print:  ret = wctype("print");  break;
			case cntrl:  ret = wctype("cntrl");  break;
			case upper:  ret = wctype("upper");  break;
			case lower:  ret = wctype("lower");  break;
			case alpha:  ret = wctype("alpha");  break;
			case digit:  ret = wctype("digit");  break;
			case punct:  ret = wctype("punct");  break;
			case xdigit: ret = wctype("xdigit"); break;
			case alnum:  ret = wctype("alnum");  break;
			case graph:  ret = wctype("graph");  break;
			default:     ret = wmask_type();
		}
		return ret;
	}
	void M_initialize_ctype()
	{
		wint_t i;
		for (i = 0; i < 128; ++i) {
			const int c = wctob(i);
			if (c == EOF)
				break;
			else
				M_narrow[i] = static_cast<char>(c);
		}
		if (i == 128)
			M_narrow_ok = true;
		else
			M_narrow_ok = false;
		for (size_t i = 0; i < sizeof(M_widen) / sizeof(wint_t); ++i)
			M_widen[i] = btowc(i);

		for (size_t i = 0; i <= 15; ++i) {
			M_bit[i] = static_cast<mask>(1 << i);
			M_wmask[i] = M_convert_to_wmask(M_bit[i]);
		}
	}
	virtual ~ascii_ctype_facet() {}
	char_type do_toupper(char_type c) const
	{
		if (c >= 0x80)
			throw ctype_failure();
		return toupper(static_cast<int>(c));
	}
	char_type const * do_toupper(char_type * lo, char_type const * hi) const
	{
		while (lo < hi) {
			if (*lo >= 0x80)
				throw ctype_failure();
			*lo = toupper(static_cast<int>(*lo));
			++lo;
		}
		return hi;
	}
	char_type do_tolower(char_type c) const
	{
		if (c >= 0x80)
			throw ctype_failure();
		return tolower(c);
	}
	char_type const * do_tolower(char_type * lo, char_type const * hi) const
	{
		while (lo < hi) {
			if (*lo >= 0x80)
				throw ctype_failure();
			*lo = tolower(*lo);
			++lo;
		}
		return hi;
	}
	bool do_is(mask m, char_type c) const
	{
		if (c >= 0x80)
			throw ctype_failure();
		// The code below works because c is in the ASCII range.
		// We could not use iswctype() which is designed for a 2byte
		// whar_t without encoding conversion otherwise.
		bool ret = false;
		// Generically, 15 (instead of 10) since we don't know the numerical
		// encoding of the various categories in /usr/include/ctype.h.
		const size_t bitmasksize = 15;
		for (size_t bitcur = 0; bitcur <= bitmasksize; ++bitcur)
			if (m & M_bit[bitcur] &&
			    iswctype(static_cast<int>(c), M_wmask[bitcur])) {
				ret = true;
				break;
			}
		return ret;
	}
	char_type const * do_is(char_type const * lo, char_type const * hi, mask * vec) const
	{
		for (;lo < hi; ++vec, ++lo) {
			if (*lo >= 0x80)
				throw ctype_failure();
			// The code below works because c is in the ASCII range.
			// We could not use iswctype() which is designed for a 2byte
			// whar_t without encoding conversion otherwise.
			// Generically, 15 (instead of 10) since we don't know the numerical
			// encoding of the various categories in /usr/include/ctype.h.
			const size_t bitmasksize = 15;
			mask m = 0;
			for (size_t bitcur = 0; bitcur <= bitmasksize; ++bitcur)
				if (iswctype(static_cast<int>(*lo), M_wmask[bitcur]))
					m |= M_bit[bitcur];
			*vec = m;
		}
		return hi;
	}
	char_type const * do_scan_is(mask m, char_type const * lo, char_type const * hi) const
	{
		while (lo < hi && !this->do_is(m, *lo))
			++lo;
		return lo;
	}
	char_type const * do_scan_not(mask m, char_type const * lo, char_type const * hi) const
	{
		while (lo < hi && this->do_is(m, *lo) != 0)
			++lo;
		return lo;
	}
	char_type do_widen(char c) const
	{
		if (static_cast<unsigned char>(c) < 0x80)
			return c;
		throw ctype_failure();
	}
	const char* do_widen(const char* lo, const char* hi, char_type* dest) const
	{
		while (lo < hi) {
			if (static_cast<unsigned char>(*lo) >= 0x80)
				throw ctype_failure();
			*dest = *lo;
			++lo;
			++dest;
		}
		return hi;
	}
	char do_narrow(char_type wc, char) const
	{
		if (wc < 0x80)
			return static_cast<char>(wc);
		throw ctype_failure();
	}
	const char_type * do_narrow(const char_type * lo, const char_type * hi, char, char * dest) const
	{
		while (lo < hi) {
			if (*lo < 0x80)
				*dest = static_cast<char>(*lo);
			else
				throw ctype_failure();
			++lo;
			++dest;
		}
		return hi;
	}
};


/// Facet for outputting numbers to odocstreams as ascii.
/// Here we simply need defining the virtual do_put functions.
class ascii_num_put_facet : public num_put<lyx::char_type, ostreambuf_iterator<lyx::char_type, char_traits<lyx::char_type> > >
{
	typedef ostreambuf_iterator<lyx::char_type, char_traits<lyx::char_type> > iter_type;
public:
	ascii_num_put_facet(size_t refs = 0) : num_put<lyx::char_type, iter_type>(refs) {}

	/// Facet for converting numbers to ascii strings.
	class string_num_put_facet : public num_put<char, basic_string<char>::iterator>
	{
	public:
		string_num_put_facet() : num_put<char, basic_string<char>::iterator>(1) {}
	};

protected:
	iter_type
	do_put(iter_type oit, ios_base & b, char_type fill, bool v) const
	{
		return do_put_helper(oit, b, fill, v);
	}

	iter_type
	do_put(iter_type oit, ios_base & b, char_type fill, long v) const
	{
		return do_put_helper(oit, b, fill, v);
	}

	iter_type
	do_put(iter_type oit, ios_base & b, char_type fill, unsigned long v) const
	{
		return do_put_helper(oit, b, fill, v);
	}

#ifdef LYX_USE_LONG_LONG
	iter_type
	do_put(iter_type oit, ios_base & b, char_type fill, long long v) const
	{
		return do_put_helper(oit, b, fill, v);
	}

	iter_type
	do_put(iter_type oit, ios_base & b, char_type fill, unsigned long long v) const
	{
		return do_put_helper(oit, b, fill, v);
	}
#endif

	iter_type
	do_put(iter_type oit, ios_base & b, char_type fill, double v) const
	{
		return do_put_helper(oit, b, fill, v);
	}

	iter_type
	do_put(iter_type oit, ios_base & b, char_type fill, long double v) const
	{
		return do_put_helper(oit, b, fill, v);
	}

	iter_type
	do_put(iter_type oit, ios_base & b, char_type fill, void const * v) const
	{
		return do_put_helper(oit, b, fill, v);
	}

private:
	template <typename ValueType>
	iter_type
	do_put_helper(iter_type oit, ios_base & b, char_type fill, ValueType v) const
	{
		if (fill >= 0x80)
			throw num_put_failure();

		streamsize const sz = b.width() > b.precision() ?
					   b.width() : b.precision();
		// 64 is large enough, unless width or precision are bigger
		streamsize const wd = (sz > 56 ? sz : 56) + 8;
		string s(wd, '\0');
		string_num_put_facet f;
		string::const_iterator cit = s.begin();
		string::const_iterator end =
			f.put(s.begin(), b, fill, v);
		for (; cit != end; ++cit, ++oit)
			*oit = *cit;

		return oit;
	}
};


/// Facet for inputting ascii representations of numbers from idocstreams.
/// Here we simply need defining the virtual do_get functions.
class ascii_num_get_facet : public num_get<lyx::char_type, istreambuf_iterator<lyx::char_type, char_traits<lyx::char_type> > >
{
	typedef istreambuf_iterator<lyx::char_type, char_traits<lyx::char_type> > iter_type;
public:
	ascii_num_get_facet(size_t refs = 0) : num_get<lyx::char_type, iter_type>(refs) {}

	/// Facet for converting ascii representation of numbers to a value.
	class string_num_get_facet : public num_get<char, basic_string<char>::iterator>
	{
	public:
		string_num_get_facet() : num_get<char, basic_string<char>::iterator>(1) {}
	};

	/// Numpunct facet defining the I/O format.
	class numpunct_facet : public numpunct<char>
	{
	public:
		numpunct_facet() : numpunct<char>(1) {}
	};

protected:
	iter_type
	do_get(iter_type iit, iter_type eit, ios_base & b,
		ios_base::iostate & err, bool & v) const
	{
		if (b.flags() & ios_base::boolalpha) {
			numpunct_facet p;
			lyx::docstring const truename = from_local8bit(p.truename());
			lyx::docstring const falsename = from_local8bit(p.falsename());
			lyx::docstring s;
			s.resize(16);
			bool ok = true;
			size_t n = 0;
			size_t const tsize = truename.size();
			size_t const fsize = falsename.size();
			for (; iit != eit; ++iit) {
				s += *iit;
				++n;
				bool true_ok = support::prefixIs(truename, s);
				bool false_ok = support::prefixIs(falsename, s);
				if (!true_ok && !false_ok) {
					++iit;
					ok = false;
					break;
				}
				if ((true_ok && n == tsize) ||
				    (false_ok && n == fsize)) {
					++iit;
					break;
				}
			}
			if (ok) {
				err = ios_base::goodbit;
				v = truename == s ? true : false;
			} else
				err = ios_base::failbit;
			if (iit == eit)
				err |= ios_base::eofbit;
			return iit;
		} else {
			long l;
			iter_type end = this->do_get(iit, eit, b, err, l);
			if (!(err & ios_base::failbit)) {
				if (l == 0)
					v = false;
				else if (l == 1)
					v = true;
				else
					err |= ios_base::failbit;
			}
			return end;
		}
	}

	iter_type
	do_get(iter_type iit, iter_type eit, ios_base & b,
		ios_base::iostate & err, long & v) const
	{
		return do_get_integer(iit, eit, b, err, v);
	}

	iter_type
	do_get(iter_type iit, iter_type eit, ios_base & b,
		ios_base::iostate & err, unsigned short & v) const
	{
		return do_get_integer(iit, eit, b, err, v);
	}

	iter_type
	do_get(iter_type iit, iter_type eit, ios_base & b,
		ios_base::iostate & err, unsigned int & v) const
	{
		return do_get_integer(iit, eit, b, err, v);
	}

	iter_type
	do_get(iter_type iit, iter_type eit, ios_base & b,
		ios_base::iostate & err, unsigned long & v) const
	{
		return do_get_integer(iit, eit, b, err, v);
	}

#ifdef LYX_USE_LONG_LONG
	iter_type
	do_get(iter_type iit, iter_type eit, ios_base & b,
		ios_base::iostate & err, long long & v) const
	{
		return do_get_integer(iit, eit, b, err, v);
	}

	iter_type
	do_get(iter_type iit, iter_type eit, ios_base & b,
		ios_base::iostate & err, unsigned long long & v) const
	{
		return do_get_integer(iit, eit, b, err, v);
	}
#endif

	iter_type
	do_get(iter_type iit, iter_type eit, ios_base & b,
		ios_base::iostate & err, float & v) const
	{
		return do_get_float(iit, eit, b, err, v);
	}

	iter_type
	do_get(iter_type iit, iter_type eit, ios_base & b,
		ios_base::iostate & err, double & v) const
	{
		return do_get_float(iit, eit, b, err, v);
	}

	iter_type
	do_get(iter_type iit, iter_type eit, ios_base & b,
		ios_base::iostate & err, long double & v) const
	{
		return do_get_float(iit, eit, b, err, v);
	}

	iter_type
	do_get(iter_type iit, iter_type eit, ios_base & b,
		ios_base::iostate & err, void * & v) const
	{
		unsigned long val;
		iter_type end = do_get_integer(iit, eit, b, err, val);
		if (!(err & ios_base::failbit))
			v = reinterpret_cast<void *>(val);
		return end;
	}

private:
	template <typename ValueType>
	iter_type
	do_get_integer(iter_type iit, iter_type eit, ios_base & b,
			ios_base::iostate & err, ValueType & v) const
	{
		string s;
		s.reserve(64);
		for (; iit != eit && isNumpunct(*iit); ++iit)
			s += static_cast<char>(*iit);
		// We add another character, not part of the numpunct facet,
		// in order to avoid setting the eofbit in the stream state,
		// which would prevent any further read. The space seems a
		// good choice here.
		s += ' ';
		string_num_get_facet f;
		f.get(s.begin(), s.end(), b, err, v);
		if (iit == eit)
		    err |= ios_base::eofbit;

		return iit;
	}

	bool isNumpunct(lyx::char_type const c) const
	{
		/// Only account for the standard numpunct "C" locale facet.
		return c == '-' || c == '+'
			|| c == 'x' || c == 'X'
			|| isHexChar(c);
	}

	template <typename ValueType>
	iter_type
	do_get_float(iter_type iit, iter_type eit, ios_base & b,
			ios_base::iostate & err, ValueType & v) const
	{
		// Gather a string of the form
		// [+-]? [0-9]* .? [0-9]* ([eE] [+-]? [0-9]+)?
		string s;
		s.reserve(64);
		numpunct_facet p;
		char const dot = p.decimal_point();
		char const sep = p.thousands_sep();
		// Get an optional sign
		if (iit != eit && (*iit == '-' || *iit == '+')) {
			s += static_cast<char>(*iit);
			++iit;
		}
		for (; iit != eit && isDigitOrSep(*iit, sep); ++iit)
			s += static_cast<char>(*iit);
		if (iit != eit && *iit == dot) {
			s += dot;
			++iit;
			for (; iit != eit && isDigitOrSep(*iit, 0); ++iit)
				s += static_cast<char>(*iit);
			if (iit != eit && (*iit == 'e' || *iit == 'E')) {
				s += static_cast<char>(*iit);
				++iit;
				for (; iit != eit && isDigitOrSep(*iit, 0); ++iit)
					s += static_cast<char>(*iit);
			}
		}
		s += '\n';
		string_num_get_facet f;
		f.get(s.begin(), s.end(), b, err, v);
		if (iit == eit)
		    err |= ios_base::eofbit;

		return iit;
	}

	bool isDigitOrSep(lyx::char_type const c, char const sep) const
	{
		return (c >= '0' && c <= '9') || (c != 0 && c == sep);
	}
};


/// class to add our facets to the global locale
class locale_initializer {
public:
	locale_initializer()
	{
		locale global;
		locale const loc1(global, new ascii_ctype_facet);
		locale const loc2(loc1, new ascii_num_put_facet);
		locale const loc3(loc2, new ascii_num_get_facet);
		locale::global(loc3);
	}
};


namespace {

/// make sure that our facets get used
static locale_initializer initializer;

}
}
#endif
