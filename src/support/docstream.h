// -*- C++ -*-
/**
 * \file docstream.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_DOCSTREAM_H
#define LYX_DOCSTREAM_H

#include "TexRow.h"
#include "support/docstring.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1600) 
// Ugly workaround for MSVC10 STL bug:
// std::numpunct has a hardcoded dllimport in definition, but we wanna it with 32 bit 
// so we can't import it and must define it but then the compiler complains.
#include "support/numpunct_lyx_char_type.h"
#endif

#include <fstream>
#include <sstream>

namespace lyx {

class iconv_codecvt_facet_exception : public std::exception {
public:
	virtual ~iconv_codecvt_facet_exception() throw() {}
	virtual const char * what() const throw();
};

/// Base class for UCS4 input streams
typedef std::basic_istream<char_type> idocstream;

/** Base class for UCS4 output streams.
    If you want to output a single UCS4 character, use \code
    os.put(c);
    \endcode, not \code
    os << c;
    \endcode . The latter will not output the character, but the code point
    as number if USE_WCHAR_T is not defined. This is because we can't overload
    operator<< (our character type is not always a real type but sometimes a
    typedef). Narrow characters of type char can be output as usual.
 */
typedef std::basic_ostream<char_type> odocstream;

/// File stream for reading UTF8-encoded files with automatic conversion to
/// UCS4.
class ifdocstream : public std::basic_ifstream<char_type> {
	typedef std::basic_ifstream<char_type> base;
public:
	ifdocstream();
	explicit ifdocstream(const char* s,
		std::ios_base::openmode mode = std::ios_base::in,
		std::string const & encoding = "UTF-8");
	~ifdocstream() {}
};

/// File stream for writing files in 8bit encoding \p encoding with automatic
/// conversion from UCS4.

class ofdocstream : public std::basic_ofstream<char_type> {
	typedef std::basic_ofstream<char_type> base;
public:
	ofdocstream();
	explicit ofdocstream(const char* s,
		std::ios_base::openmode mode = std::ios_base::out|std::ios_base::trunc,
		std::string const & encoding = "UTF-8");
	~ofdocstream() {}
	///
	void reset(std::string const & encoding);
};



/// UCS4 input stringstream
typedef std::basic_istringstream<char_type> idocstringstream;

/// UCS4 output manipulator
typedef odocstream & (*odocstream_manip)(odocstream &);

/** Wrapper class for odocstream.
    This class is used to automatically count the lines of the exported latex
    code and also to ensure that no blank lines may be inadvertently output.
    To this end, use the special variables "breakln" and "safebreakln" as if
    they were iomanip's to ensure that the next output will start at the
    beginning of a line. Using "breakln", a '\n' char will be output if needed,
    while using "safebreakln", "%\n" will be output if needed.
    The class also records the last output character and can tell whether
    a paragraph break was just output.
  */

class otexstream {
public:
	///
	otexstream(odocstream & os, TexRow & texrow)
		: os_(os), texrow_(texrow), canbreakline_(false),
		  protectspace_(false), parbreak_(true), lastchar_(0) {}
	///
	odocstream & os() { return os_; }
	///
	TexRow & texrow() { return texrow_; }
	///
	void put(char_type const & c);
	///
	void canBreakLine(bool breakline) { canbreakline_ = breakline; }
	///
	bool canBreakLine() const { return canbreakline_; }
	///
	void protectSpace(bool protectspace) { protectspace_ = protectspace; }
	///
	bool protectSpace() const { return protectspace_; }
	///
	void lastChar(char_type const & c)
	{
		parbreak_ = (!canbreakline_ && c == '\n');
		canbreakline_ = (c != '\n');
		lastchar_ = c;
	}
	///
	char_type lastChar() const { return lastchar_; }
	///
	bool afterParbreak() const { return parbreak_; }
private:
	///
	odocstream & os_;
	///
	TexRow & texrow_;
	///
	bool canbreakline_;
	///
	bool protectspace_;
	///
	bool parbreak_;
	///
	char_type lastchar_;
};

/// Helper structs for breaking a line
struct BreakLine {
	char n;
};

struct SafeBreakLine {
	char n;
};

extern BreakLine breakln;
extern SafeBreakLine safebreakln;

///
otexstream & operator<<(otexstream &, BreakLine);
///
otexstream & operator<<(otexstream &, SafeBreakLine);
///
otexstream & operator<<(otexstream &, odocstream_manip);
///
otexstream & operator<<(otexstream &, docstring const &);
///
otexstream & operator<<(otexstream &, std::string const &);
///
otexstream & operator<<(otexstream &, char const *);
///
otexstream & operator<<(otexstream &, char);
///
template <typename Type>
otexstream & operator<<(otexstream & ots, Type value);

/// Helper struct for changing stream encoding
struct SetEnc {
	SetEnc(std::string const & e) : encoding(e) {}
	std::string encoding;
};

/// Helper function for changing stream encoding
SetEnc setEncoding(std::string const & encoding);

/** Change the encoding of \p os to \p e.encoding.
    \p e.encoding must be a valid iconv name of an 8bit encoding.
    This does nothing if the stream is not a file stream, since only
    file streams do have an associated 8bit encoding.
    Usage: \code
    os << setEncoding("ISO-8859-1");
    \endcode
 */
odocstream & operator<<(odocstream & os, SetEnc e);
idocstream & operator<<(idocstream & os, SetEnc e);

}

#endif
