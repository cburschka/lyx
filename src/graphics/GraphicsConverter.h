// -*- C++ -*-
/**
 * \file GraphicsConverter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * The controller of a conversion process from file AA of format A to
 * file BB of format B.
 * Once finished, a signal is emitted to inform any listeners (connected
 * through the connect() method).
 */

#ifndef GRAPHICSCONVERTER_H
#define GRAPHICSCONVERTER_H

#include <boost/scoped_ptr.hpp>
#include <boost/signal.hpp>
#include <boost/noncopyable.hpp>

namespace lyx {

namespace support { class FileName; }

namespace graphics {

class Converter : boost::noncopyable {
public:
	/// Can the conversion be performed?
	static bool isReachable(std::string const & from_format_name,
				std::string const & to_format_name);

	/** One Converter per conversion ensures that the (hidden) signal
	 *  is always connected to the expected slot.
	 */
	Converter(support::FileName const & from_file, std::string const & to_file_base,
		  std::string const & from_format, std::string const & to_format);

	/// Define an empty d-tor out-of-line to keep boost::scoped_ptr happy.
	~Converter();

	/// We are explicit about when we begin the conversion process.
	void startConversion() const;

	/** Connect and you'll be informed when the conversion process has
	 *  finished.
	 *  If the conversion is succesful, then the listener is passed \c true.
	 */
	typedef boost::signal<void(bool)> sig_type;
	typedef sig_type::slot_type slot_type;
	///
	boost::signals::connection connect(slot_type const &) const;

	/** If the conversion is succesful, this returns the name of the
	 *  resulting file.
	 *  If conversion fails or has not been completed, however, it
	 *  returns an empty string.
	 */
	support::FileName const & convertedFile() const;

private:
	/// Use the Pimpl idiom to hide the internals.
	class Impl;

	/// The pointer never changes although *pimpl_'s contents may.
	boost::scoped_ptr<Impl> const pimpl_;
};

} // namespace graphics
} // namespace lyx

#endif // GRAPHICSCONVERTER_H
