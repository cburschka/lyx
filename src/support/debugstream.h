// -*- C++ -*-
/**
 * \file debugstream.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DEBUG_STREAM_HPP
#define DEBUG_STREAM_HPP

#include <iostream>

#include <boost/version.hpp>


//namespace lyx {

#if BOOST_VERSION < 103300
#  include <boost/test/detail/nullstream.hpp>
#else
#  include <boost/test/utils/nullstream.hpp>
#endif

#ifdef DEBUG
# define TEMPORARY_DEBUG_MACRO DEBUG
# undef DEBUG
#endif

#ifdef TEMPORARY_DEBUG_MACRO
# define DEBUG TEMPORARY_DEBUG_MACRO
# undef TEMPORARY_DEBUG_MACRO
#endif


template <class dtrait,
	  class charT = char,
	  class traits = std::char_traits<charT> >
class basic_debugstream : public std::basic_ostream<charT, traits> {
public:
	typedef dtrait debug;
	typedef typename debug::type Type;

	basic_debugstream()
		: std::basic_ostream<charT, traits>(0), dt(debug::NONE),
		  realbuf_(0), enabled_(true)
	{}

	/// Constructor, sets the debug level to t.
	explicit basic_debugstream(std::basic_streambuf<charT, traits> * buf)
		: std::basic_ostream<charT, traits>(buf), dt(debug::NONE),
		  realbuf_(0), enabled_(true)
	{}

	/// Sets the debug level to t.
	void level(Type t) {
		dt = t;
	}

	/// Returns the current debug level.
	Type level() const {
		return dt;
	}

	/// Returns true if t is part of the current debug level.
	bool debugging(Type t = debug::ANY) const
	{
		if (debug::match(dt, t)) return true;
		return false;
	}

	/** Returns the no-op stream if t is not part of the
	    current debug level otherwise the real debug stream
	    is used.
	    Use: dbgstream[Debug::INFO] << "Info!\n";
	*/
	std::basic_ostream<charT, traits> & operator[](Type t) {
		if (debug::match(dt, t))
			return *this;
		return nullstream;
	}
	/// Disable the stream completely
	void disable()
	{
		if (enabled_) {
			realbuf_ = this->rdbuf();
			rdbuf(nullstream.rdbuf());
			enabled_ = false;
		}
	}
	/// Enable the stream after a possible call of disable()
	void enable()
	{
		if (!enabled_) {
			this->rdbuf(realbuf_);
			enabled_ = true;
		}
	}
private:
	/// The current debug level
	Type dt;
	/// The no-op stream.
	boost::basic_onullstream<charT, traits> nullstream;
	/// The buffer of the real stream
	std::streambuf * realbuf_;
	/// Is the stream enabled?
	bool enabled_;
};


//} // namespace lyx

#endif
