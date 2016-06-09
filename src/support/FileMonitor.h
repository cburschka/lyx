// -*- C++ -*-
/**
 * \file FileMonitor.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * FileMonitor monitors a file and informs a listener when that file has
 * changed.
 */

#ifndef FILEMONITOR_H
#define FILEMONITOR_H

#include <boost/signals2.hpp>

namespace lyx {
namespace support {

class FileName;

class FileMonitor
{
public:
	/** Once monitoring begins, the file will be monitored every
	 *  interval ms.
	 *
	 * FIXME: rewrite and simplify using an encapsulation of QFileSystemWatcher.
	 */
	FileMonitor(FileName const & file_with_path, int interval);

	/// Destructor
	~FileMonitor();

	///
	void reset(FileName const & file_with_path) const;

	///
	FileName const & filename() const;

	/// Begin monitoring the file
	void start() const;
	///
	void stop() const;
	///
	bool monitoring() const;

	/** The checksum is recomputed whenever the file is modified.
	 *  If the file is not being monitored, then the checksum will be
	 *  recomputed each time this function is called.
	 */
	unsigned long checksum() const;

	/// Connect and you'll be informed when the file has changed.
	typedef boost::signals2::signal<void()> FileChangedSig;
	typedef FileChangedSig::slot_type slot_type;
	///
	boost::signals2::connection connect(slot_type const &) const;

private:
	/// noncopyable
	FileMonitor(FileMonitor const &);
	void operator=(FileMonitor const &);

	/// Use the Pimpl idiom to hide the internals.
	class Impl;
	/// The pointer never changes although *pimpl_'s contents may.
	Impl * const pimpl_;
};

} // namespace support
} // namespace lyx

#endif // FILEMONITOR_H
