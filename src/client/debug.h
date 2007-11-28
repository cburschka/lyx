// -*- C++ -*-
/**
 * \file debug.h
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

#include "support/docstring.h"


namespace lyx {

/** Ideally this should have been a namespace, but since we try to be
 *  compilable on older C++ compilators too, we use a struct instead.
 *  This is all the different debug levels that we have.
 */
class Debug {
public:
	///
	enum Type {
		///
		NONE = 0,
		///
		INFO       = (1 << 0),
		///
		DEBUG      = (1 << 31),
		///
		ANY = 0xffffffff
	};

	/** A function to convert symbolic string names on debug levels
	    to their numerical value.
	*/
	static Type value(std::string const & val);

	/** Display the tags and descriptions of the current debug level
	    of ds
	*/
	static void showLevel(std::ostream & o, Type level);

	/** show all the possible tags that can be used for debugging */
	static void showTags(std::ostream & o);

};


inline void operator|=(Debug::Type & d1, Debug::Type d2)
{
	d1 = static_cast<Debug::Type>(d1 | d2);
}


class LyXErr
{
public:
	/// Disable the stream completely
	void disable();
	/// Enable the stream after a possible call of disable()
	void enable();
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
LyXErr & operator<<(LyXErr &, std::ostream &(*)(std::ostream &));
LyXErr & operator<<(LyXErr &, std::ios_base &(*)(std::ios_base &));
LyXErr & operator<<(LyXErr & l, support::FileName const & t);

extern LyXErr lyxerr;

} // namespace lyx

#define LYXERR(type, msg) \
	do { if (!lyx::lyxerr.debugging(type)) {} \
       else lyx::lyxerr << msg  << std::endl; } while (0)


#endif
