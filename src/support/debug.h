// -*- C++ -*-
/**
 * \file debug.h
 *
 * FIXME: It would be nice if, in lyx::use_gui mode, instead of
 * outputting to the console, we would pipe all messages onto a file
 * and visualise the contents dynamically in a Qt window if needed.
 *
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXDEBUG_H
#define LYXDEBUG_H

#include "support/strfwd.h"


namespace std {

class ios_base;

template<typename CharT, typename Traits> class basic_streambuf;
typedef basic_streambuf<char, char_traits<char> > streambuf;

}


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
		INSETTEXT  = (1 << 20),
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
		///
		DEBUG      = (1 << 31),
		///
		ANY = 0xffffffff
	};

	/** A function to convert symbolic string names on debug levels
	    to their numerical value.
	*/
	Type value(std::string const & val);

	/** Display the tags and descriptions of the current debug level
	    of ds
	*/
	void showLevel(std::ostream & os, Type level);

	/** show all the possible tags that can be used for debugging */
	void showTags(std::ostream & os);

} // namespace Debug


inline void operator|=(Debug::Type & d1, Debug::Type d2)
{
	d1 = static_cast<Debug::Type>(d1 | d2);
}


class LyXErr
{
public:
	LyXErr(): enabled_(true) {}
	/// Disable the stream completely
	void disable();
	/// Enable the stream after a possible call of disable()
	void enable();
	///
	bool enabled() const { return enabled_; }
	/// Returns true if t is part of the current debug level.
	bool debugging(Debug::Type t = Debug::ANY) const;
	/// Ends output
	void endl();
	/// Sets stream
	void setStream(std::ostream & os) { stream_ = &os; }
	/// Sets stream
	std::ostream & stream() { return *stream_; }
	/// Sets the debug level to t.
	void level(Debug::Type t) { dt = t; }
	/// Returns the current debug level.
	Debug::Type level() const { return dt; }
	/// Returns stream
	operator std::ostream &() { return *stream_; }
private:
	/// The current debug level
	Debug::Type dt;
	/// Is the stream enabled?
	bool enabled_;
	/// The real stream
	std::ostream * stream_;
};

namespace support { class FileName; }

LyXErr & operator<<(LyXErr &, void const *);
LyXErr & operator<<(LyXErr &, char const *);
LyXErr & operator<<(LyXErr &, char);
LyXErr & operator<<(LyXErr &, int);
LyXErr & operator<<(LyXErr &, unsigned int);
LyXErr & operator<<(LyXErr &, long);
LyXErr & operator<<(LyXErr &, unsigned long);
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
# define CURRENT_POSITION __FILE__ << "(" << __LINE__ << "): "
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
