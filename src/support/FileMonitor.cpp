/**
 * \file FileMonitor.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/FileMonitor.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/qstring_helpers.h"
#include "support/unique_ptr.h"

#include <QFile>
#include <QStringList>
#include <QTimer>

#include <algorithm>

using namespace std;

namespace lyx {
namespace support {


FileSystemWatcher & FileSystemWatcher::instance()
{
	// This is thread-safe because QFileSystemWatcher is thread-safe.
	static FileSystemWatcher f;
	return f;
}


FileSystemWatcher::FileSystemWatcher()
	: qwatcher_(make_unique<QFileSystemWatcher>())
{}


shared_ptr<FileMonitorGuard>
FileSystemWatcher::getGuard(FileName const & filename)
{
	string const absfilename = filename.absFileName();
	weak_ptr<FileMonitorGuard> & wptr = store_[absfilename];
	if (shared_ptr<FileMonitorGuard> mon = wptr.lock())
		return mon;
	auto mon = make_shared<FileMonitorGuard>(absfilename, qwatcher_.get());
	wptr = mon;
	return mon;
}


//static
FileMonitorPtr FileSystemWatcher::monitor(FileName const & filename)
{
	return make_unique<FileMonitor>(instance().getGuard(filename));
}


//static
ActiveFileMonitorPtr FileSystemWatcher::activeMonitor(FileName const & filename,
                                                      int interval)
{
	return make_unique<ActiveFileMonitor>(instance().getGuard(filename),
	                                      filename, interval);
}


//static
void FileSystemWatcher::debug()
{
	FileSystemWatcher & f = instance();
	QStringList q_files = f.qwatcher_->files();
	for (pair<string, weak_ptr<FileMonitorGuard>> pair : f.store_) {
		string const & name = pair.first;
		if (!pair.second.expired()) {
			if (!q_files.contains(toqstr(name)))
				LYXERR0("Monitored but not QFileSystemWatched (bad): " << name);
			else {
				//LYXERR0("Monitored and QFileSystemWatched (good): " << name);
			}
		}
	}
	for (QString const & qname : q_files) {
		string const name = fromqstr(qname);
		weak_ptr<FileMonitorGuard> & wptr = f.store_[name];
		if (wptr.expired())
			LYXERR0("QFileSystemWatched but not monitored (bad): " << name);
	}
}


FileMonitorGuard::FileMonitorGuard(string const & filename,
                                   QFileSystemWatcher * qwatcher)
	: filename_(filename), qwatcher_(qwatcher), exists_(true)
{
	QObject::connect(qwatcher, SIGNAL(fileChanged(QString const &)),
	                 this, SLOT(notifyChange(QString const &)));
	if (qwatcher_->files().contains(toqstr(filename)))
		LYXERR0("This file is already being QFileSystemWatched: " << filename
		        << ". This should not happen.");
	refresh();
}


FileMonitorGuard::~FileMonitorGuard()
{
	qwatcher_->removePath(toqstr(filename_));
}


void FileMonitorGuard::refresh()
{
	QString const qfilename = toqstr(filename_);
	if(!qwatcher_->files().contains(qfilename)) {
		bool exists = QFile(qfilename).exists();
#if (QT_VERSION >= 0x050000)
		if (!exists || !qwatcher_->addPath(qfilename))
#else
		auto add_path = [&]() {
			qwatcher_->addPath(qfilename);
			return qwatcher_->files().contains(qfilename);
		};
		if (!exists || !add_path())
#endif
		{
			if (exists)
				LYXERR(Debug::FILES,
				       "Could not add path to QFileSystemWatcher: "
				       << filename_);
			QTimer::singleShot(2000, this, SLOT(refresh()));
		} else if (exists && !exists_)
			Q_EMIT fileChanged();
		setExists(exists);
	}
}


void FileMonitorGuard::notifyChange(QString const & path)
{
	if (path == toqstr(filename_)) {
		Q_EMIT fileChanged();
		// If the file has been modified by delete-move, we are notified of the
		// deletion but we no longer track the file. See
		// <https://bugreports.qt.io/browse/QTBUG-46483> (not a bug).
		refresh();
	}
}


FileMonitor::FileMonitor(std::shared_ptr<FileMonitorGuard> monitor)
	: monitor_(monitor)
{
	QObject::connect(monitor_.get(), SIGNAL(fileChanged()),
	                 this, SLOT(changed()));
	refresh();
}


void FileMonitor::reconnectToFileMonitorGuard()
{
	monitor_->setExists(true);
	QObject::connect(monitor_.get(), SIGNAL(fileChanged()),
	                 this, SLOT(changed()));
}


boost::signals2::connection
FileMonitor::connect(sig::slot_type const & slot)
{
	return fileChanged_.connect(slot);
}


void FileMonitor::disconnect()
{
	fileChanged_.disconnect_all_slots();
	QObject::disconnect(this, SIGNAL(fileChanged()));
}


void FileMonitor::changed()
{
	// emit boost signal
	fileChanged_();
	Q_EMIT fileChanged();
}


FileMonitorBlocker FileMonitor::block(int delay)
{
	FileMonitorBlocker blocker = blocker_.lock();
	if (!blocker)
		blocker_ = blocker = make_shared<FileMonitorBlockerGuard>(this);
	blocker->setDelay(delay);
	return blocker;
}


FileMonitorBlockerGuard::FileMonitorBlockerGuard(FileMonitor * monitor)
	: monitor_(monitor), delay_(0)
{
	QObject::disconnect(monitor->monitor_.get(), SIGNAL(fileChanged()),
	                    monitor, SLOT(changed()));
}


void FileMonitorBlockerGuard::setDelay(int delay)
{
	delay_ = max(delay_, delay);
}


FileMonitorBlockerGuard::~FileMonitorBlockerGuard()
{
	if (!monitor_)
		return;
	// Even if delay_ is 0, the QTimer is necessary. Indeed, the notifications
	// from QFileSystemWatcher that we meant to ignore are not going to be
	// treated immediately, so we must yield to give us the opportunity to
	// ignore them.
	QTimer::singleShot(delay_, monitor_, SLOT(reconnectToFileMonitorGuard()));
}


ActiveFileMonitor::ActiveFileMonitor(std::shared_ptr<FileMonitorGuard> monitor,
                                     FileName const & filename, int interval)
	: FileMonitor(monitor), filename_(filename), interval_(interval),
	  timestamp_(0), checksum_(0), cooldown_(true)
{
	QObject::connect(this, SIGNAL(fileChanged()), this, SLOT(setCooldown()));
	QTimer::singleShot(interval_, this, SLOT(clearCooldown()));
	if (!filename_.exists())
		return;
	timestamp_ = filename_.lastModified();
	checksum_ = filename_.checksum();
}


void ActiveFileMonitor::checkModified()
{
	if (cooldown_)
		return;

	cooldown_ = true;
	bool changed = false;
	if (!filename_.exists()) {
		changed = timestamp_ || checksum_;
		timestamp_ = 0;
		checksum_ = 0;
	} else {
		time_t const new_timestamp = filename_.lastModified();

		if (new_timestamp != timestamp_) {
			timestamp_ = new_timestamp;

			unsigned long const new_checksum = filename_.checksum();
			if (new_checksum != checksum_) {
				checksum_ = new_checksum;
				changed = true;
			}
		}
	}
	if (changed)
		FileMonitor::changed();
	QTimer::singleShot(interval_, this, SLOT(clearCooldown()));
}


void ActiveFileMonitor::checkModifiedAsync()
{
	if (!cooldown_)
		QTimer::singleShot(0, this, SLOT(checkModified()));
}



} // namespace support
} // namespace lyx

#include "moc_FileMonitor.cpp"
