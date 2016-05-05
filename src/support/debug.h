// -*- C++ -*-
/**
 * \file debug.h
 *
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Pavel Sanda
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXDEBUG_H
#define LYXDEBUG_H

#include "support/strfwd.h"

// Forward definitions do not work with libc++
// but ios_base has already been defined in strfwd
// if compiling with it
#ifndef USE_LLVM_LIBCPP
namespace std {

class ios_base;

template<typename CharT, typename Traits> class basic_streambuf;
typedef basic_streambuf<char, char_traits<char> > streambuf;

}
#endif


namespace lyx {

///  This is all the different debug levels that we have.
namespace Debug {
	///
	enum Type {
		///
		NONE = 0,
		///
		INFO       = (1 << 0),   // 1
		///
		INIT       = (1 << 1),   // 2
		///
		KEY        = (1 << 2),   // 4
		///
		GUI        = (1 << 3),   // 8
		///
		PARSER     = (1 << 4),   // 16
		///
		LYXRC      = (1 << 5),   // 32
		///
		KBMAP      = (1 << 6),   // 64
		///
		LATEX      = (1 << 7),   // 128
		///
		MATHED     = (1 << 8),   // 256 // Alejandro, please use this.
		///
		FONT       = (1 << 9),   // 512
		///
		TCLASS     = (1 << 10),  // 1024
		///
		LYXVC      = (1 << 11),  // 2048
		///
		LYXSERVER  = (1 << 12),  // 4096
		///
		UNDO       = (1 << 13),  // 8192
		///
		ACTION     = (1 << 14),   // 16384
		///
		LYXLEX     = (1 << 15),
		///
		DEPEND     = (1 << 16),
		///
		INSETS     = (1 << 17),
		///
		FILES      = (1 << 18),
		///
		WORKAREA   = (1 << 19),
		///
		CLIPBOARD  = (1 << 20),
		///
		GRAPHICS   = (1 << 21),
		/// change tracking
		CHANGES    = (1 << 22),
		///
		EXTERNAL   = (1 << 23),
		///
		PAINTING   = (1 << 24),
		///
		SCROLLING  = (1 << 25),
		///
		MACROS     = (1 << 26),
		///	rtl-related
		RTL        = (1 << 27),
		///	locale related
		LOCALE     = (1 << 28),
		///	selection
		SELECTION  = (1 << 29),
		/// Find and Replace
		FIND       = (1 << 30),
		///
		DEBUG      = (1 << 31),
		///
		ANY = 0xffffffff
	};

	/// Return number of levels
	int levelCount();
 
	/// A function to convert debug level string names numerical values
	Type value(std::string const & val);

	/// A function to convert index of level to their numerical value
	Type value(int val);

	/// Return description of level
	std::string const description(Type val);

	/// Return name of level
	std::string const name(Type val);

	/// Display the tags and descriptions of the current debug level
	void showLevel(std::ostream & os, Type level);

	/// Show all the possible tags that can be used for debugging
	void showTags(std::ostream & os);


} // namespace Debug


inline void operator|=(Debug::Type & d1, Debug::Type d2)
{
	d1 = static_cast<Debug::Type>(d1 | d2);
}


class LyXErr
{
public:
	LyXErr(): dt_(Debug::NONE), stream_(0), enabled_(true),
	          second_stream_(0), second_enabled_(false) {}
	
	/// Disable the stream completely
	void disable();
	/// Enable the stream after a possible call of disable()
	void enable();

	/// Ends output
	void endl();

	/// Returns stream
	std::ostream & stream() { return *stream_; }
	/// Returns stream
	operator std::ostream &() { return *stream_; }
	/// Sets stream
	void setStream(std::ostream & os) { stream_ = &os; }
	/// Is the stream enabled ?
	bool enabled() const { return enabled_; }

	/// Returns second stream
	std::ostream & secondStream() { return *second_stream_; }
	/// Sets second stream
	void setSecondStream(std::ostream * os) 
		{ second_enabled_ = (second_stream_ = os); }
	/// Is the second stream is enabled?
	bool secondEnabled() { return second_enabled_; }

	/// Sets the debug level
	void setLevel(Debug::Type t) { dt_ = t; }
	/// Returns the current debug level
	Debug::Type level() const { return dt_; }
	/// Returns true if t is part of the current debug level
	bool debugging(Debug::Type t = Debug::ANY) const;

	///
	static char const * stripName(char const *);

private:
	/// The current debug level
	Debug::Type dt_;
	/// The real stream
	std::ostream * stream_;
	/// Is the stream enabled?
	bool enabled_;
	/// Next stream for output duplication
	std::ostream * second_stream_;
	/// Is the second stream enabled?
	bool second_enabled_;
};

namespace support { class FileName; }

LyXErr & operator<<(LyXErr &, void const *);
LyXErr & operator<<(LyXErr &, char const *);
LyXErr & operator<<(LyXErr &, char);
LyXErr & operator<<(LyXErr &, int);
LyXErr & operator<<(LyXErr &, unsigned int);
LyXErr & operator<<(LyXErr &, long);
LyXErr & operator<<(LyXErr &, unsigned long);
#ifdef LYX_USE_LONG_LONG
LyXErr & operator<<(LyXErr &, long long);
LyXErr & operator<<(LyXErr &, unsigned long long);
#endif
LyXErr & operator<<(LyXErr &, double);
LyXErr & operator<<(LyXErr &, std::string const &);
LyXErr & operator<<(LyXErr &, docstring const &);
LyXErr & operator<<(LyXErr &, support::FileName const &);
LyXErr & operator<<(LyXErr &, std::ostream &(*)(std::ostream &));
LyXErr & operator<<(LyXErr &, std::ios_base &(*)(std::ios_base &));

extern LyXErr lyxerr;

} // namespace lyx

#if USE_BOOST_CURRENT_FUNCTION
#	include <boost/current_function.hpp>
#	define CURRENT_POSITION BOOST_CURRENT_FUNCTION ": "
#else
# define CURRENT_POSITION lyx::LyXErr::stripName(__FILE__) << " (" << __LINE__ << "): "
#endif

#define LYXERR(type, msg) \
	do { \
		if (!lyx::lyxerr.debugging(type)) {} \
		else { lyx::lyxerr << CURRENT_POSITION << msg; lyx::lyxerr.endl(); } \
	} while (0)

#define LYXERR0(msg) \
	do { \
		lyx::lyxerr << CURRENT_POSITION << msg; lyx::lyxerr.endl(); \
	} while (0)


#endif
