// -*- C++ -*-
/**
 *  \file GraphicsConverter.h
 *  Read the file COPYING
 *
 *  \author Angus Leeming 
 *
 * Full author contact details available in file CREDITS
 *
 *  The controller of a conversion process from file AA of format A to
 *  file BB of format B.
 *  Once finished, a signal is emitted to inform any listeners (connected
 *  through the connect() method).
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

	/** One Converter per conversion ensures that the (hidden) signal
	 *  is always connected to the expected slot.
	 */
	Converter(string const & from_file,   string const & to_file_base,
		  string const & from_format, string const & to_format);

	/// Define an empty d-tor out-of-line to keep boost::scoped_ptr happy.
	~Converter();

	/// We are explicit about when we begin the conversion process.
	void startConversion() const;

	/** Connect and you'll be informed when the conversion process has
	 *  finished.
	 *  If the conversion is succesful, then the listener is passed \c true.
	 */
	typedef boost::signal1<void, bool>::slot_type slot_type;
	///
	boost::signals::connection connect(slot_type const &) const;

	/** If the conversion is succesful, this returns the name of the
	 *  resulting file.
	 *  If conversion fails or has not been completed, however, it
	 *  returns an empty string.
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
