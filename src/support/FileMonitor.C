/*
 * \file FileMonitor.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming <leeming@lyx.org>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FileMonitor.h"

#include "frontends/Timeout.h"

#include "support/FileInfo.h"
#include "support/lyxlib.h"

#include <boost/bind.hpp>
#include <boost/signals/trackable.hpp>

struct FileMonitor::Impl : public boost::signals::trackable {

	///
	Impl(string const & file_with_path, int interval);

	///
	void monitorFile();

	///
	string filename_;

	///
	Timeout timer_;

	/// This signal is emitted if the file is modified (has a new checksum).
	boost::signal0<void> fileChanged_;

	/** We use these to ascertain whether a file (once loaded successfully)
	 *  has changed.
	 */
	time_t timestamp_;
	///
	unsigned long checksum_;
};


FileMonitor::FileMonitor(string const & file_with_path, int interval)
	: pimpl_(new Impl(file_with_path, interval))
{}


FileMonitor::~FileMonitor()
{}


void FileMonitor::reset(string const & file_with_path) const
{
	if (pimpl_->filename_ == file_with_path)
		return;

	bool const monitor = pimpl_->timer_.running();
	if (monitor)
		stop();
	
	pimpl_->filename_ = file_with_path;

	if (monitor)
		start();
}


string const & FileMonitor::filename() const
{
	return pimpl_->filename_;
}


void FileMonitor::start() const
{
	if (monitoring())
		return;

	FileInfo finfo(pimpl_->filename_);
	if (!finfo.isOK())
		return;

	pimpl_->timestamp_ = finfo.getModificationTime();
	pimpl_->checksum_ = lyx::sum(pimpl_->filename_);

	if (pimpl_->timestamp_ && pimpl_->checksum_) {
		pimpl_->timer_.start();
	} else {
		pimpl_->timestamp_ = 0;
		pimpl_->checksum_ = 0;
	}
}


void FileMonitor::stop() const
{
	pimpl_->timestamp_ = 0;
	pimpl_->checksum_ = 0;
	pimpl_->timer_.stop();
}


bool FileMonitor::monitoring() const
{
	return pimpl_->timer_.running();
}


unsigned long FileMonitor::checksum() const
{
	// If we aren't actively monitoring the file, then recompute the
	// checksum explicitly.
	if (!pimpl_->timer_.running() && !pimpl_->filename_.empty())
		return lyx::sum(pimpl_->filename_);

	return pimpl_->checksum_;
}


boost::signals::connection FileMonitor::connect(slot_type const & slot) const
{
	return pimpl_->fileChanged_.connect(slot);
}


//------------------------------
// Implementation details follow
//------------------------------


FileMonitor::Impl::Impl(string const & file_with_path, int interval)
	: filename_(file_with_path),
	  timer_(interval, Timeout::ONETIME),
	  timestamp_(0),
	  checksum_(0)
{
	timer_.timeout.connect(boost::bind(&Impl::monitorFile, this));
}


void FileMonitor::Impl::monitorFile()
{
	bool changed = false;

	FileInfo finfo(filename_);
	if (!finfo.isOK()) {
		changed = timestamp_ || checksum_;
		timestamp_ = 0;
		checksum_ = 0;

	} else {
		time_t const new_timestamp = finfo.getModificationTime();

		if (new_timestamp != timestamp_) {
			timestamp_ = new_timestamp;

			unsigned long const new_checksum = lyx::sum(filename_);
			if (new_checksum != checksum_) {
				checksum_ = new_checksum;
				changed = true;
			}
		}
	}

	timer_.start();
	if (changed)
		fileChanged_();
}
