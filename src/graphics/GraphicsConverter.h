// -*- C++ -*-
/**
 *  \file GraphicsConverter.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 *  \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 *  The controller of a conversion process from file AA of format A to
 *  file BB of format B.
 *  Once finished, the signal finishdConversion is emitted to inform the
 *  instigator where to find file BB.
 *  If the conversion is unsuccessful, then finishedConversion will pass
 *  an empty string.
 */

#ifndef GRAPHICSCONVERTER_H
#define GRAPHICSCONVERTER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include <boost/signals/signal1.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

namespace grfx {

class Converter : boost::noncopyable {
public:
	/// Can the conversion be performed?
	static bool isReachable(string const & from_format_name,
				string const & to_format_name);

	/** One Converter per conversion ensures that finishedConversion
	 *  is always connected to the expected slot.
	 */
	Converter(string const & from_file,   string const & to_file_base,
		  string const & from_format, string const & to_format);

	/// Define an empty d-tor out-of-line to keep boost::scoped_ptr happy.
	~Converter();

	/// We are explicit about when we begin the conversion process.
	void startConversion();

	/** At the end of the conversion process inform the outside world
	 *  by emitting a signal.
	 */
	typedef boost::signal1<void, bool> SignalType;
	///
	SignalType finishedConversion;
	
	/** If the convsion is succesful (finishedConversion returns \c true),
	 *  this returns the name of the resulting file.
	 *  If conversion fails, however, it returns an empty string.
	 */
	string const & convertedFile() const;

private:
	/// Use the Pimpl idiom to hide the internals.
	class Impl;

	/// The pointer never changes although *pimpl_'s contents may.
	boost::scoped_ptr<Impl> const pimpl_;
};
 
} // namespace grfx

#endif // GRAPHICSCONVERTER_H
