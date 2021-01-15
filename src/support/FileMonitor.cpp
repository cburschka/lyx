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
	return lyx::make_unique<FileMonitor>(instance().getGuard(filename));
}


//static
ActiveFileMonitorPtr FileSystemWatcher::activeMonitor(FileName const & filename,
                                                      int interval)
{
	return lyx::make_unique<ActiveFileMonitor>(instance().getGuard(filename),
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
	if (filename.empty())
		return;
	QObject::connect(qwatcher, SIGNAL(fileChanged(QString const &)),
	                 this, SLOT(notifyChange(QString const &)));
	if (qwatcher_->files().contains(toqstr(filename)))
		LYXERR0("This file is already being QFileSystemWatched: " << filename
		        << ". This should not happen.");
	refresh();
}


FileMonitorGuard::~FileMonitorGuard()
{
	if (!filename_.empty())
		qwatcher_->removePath(toqstr(filename_));
}


void FileMonitorGuard::refresh(bool const emit)
{
	if (filename_.empty())
		return;
	QString const qfilename = toqstr(filename_);
	if (!qwatcher_->files().contains(qfilename)) {
		bool const existed = exists_;
		exists_ = QFile(qfilename).exists();
#if (QT_VERSION >= 0x050000)
		if (exists_ && !qwatcher_->addPath(qfilename))
#else
		auto add_path = [&]() {
			qwatcher_->addPath(qfilename);
			return qwatcher_->files().contains(qfilename);
		};
		if (exists_ && !add_path())
#endif
		{
			LYXERR(Debug::FILES,
			       "Could not add path to QFileSystemWatcher: " << filename_);
			QTimer::singleShot(5000, this, SLOT(refresh()));
		} else {
			if (!exists_)
				// The standard way to overwrite a file is to delete it and
				// create a new file with the same name. Therefore if the file
				// has just been deleted, it is smart to check not too long
				// after whether it has been recreated.
			    QTimer::singleShot(existed ? 100 : 2000, this, SLOT(refresh()));
			if (existed != exists_ && emit)
				Q_EMIT fileChanged(exists_);
		}
	}
}


void FileMonitorGuard::notifyChange(QString const & path)
{
	if (path == toqstr(filename_)) {
		// If the file has been modified by delete-move, we are notified of the
		// deletion but we no longer track the file. See
		// <https://bugreports.qt.io/browse/QTBUG-46483> (not a bug).
		// Therefore we must refresh.
		refresh(false);
		Q_EMIT fileChanged(exists_);
	}
}


FileMonitor::FileMonitor(std::shared_ptr<FileMonitorGuard> monitor)
	: monitor_(monitor)
{
	connectToFileMonitorGuard();
	refresh();
}


void FileMonitor::connectToFileMonitorGuard()
{
	// Connections need to be asynchronous because the receiver can own this
	// object and therefore is allowed to delete it.
	// Qt signal:
	QObject::connect(monitor_.get(), SIGNAL(fileChanged(bool)),
	                 this, SIGNAL(fileChanged(bool)),
	                 Qt::QueuedConnection);
	// Boost signal:
	QObject::connect(this, SIGNAL(fileChanged(bool)),
	                 this, SLOT(changed(bool)));
}


connection FileMonitor::connect(slot const & slot)
{
	return fileChanged_.connect(slot);
}


void FileMonitor::changed(bool const exists)
{
	// emit boost signal
	fileChanged_(exists);
}


ActiveFileMonitor::ActiveFileMonitor(std::shared_ptr<FileMonitorGuard> monitor,
                                     FileName const & filename, int interval)
	: FileMonitor(monitor), filename_(filename), interval_(interval),
	  timestamp_(0), checksum_(0), cooldown_(true)
{
	QObject::connect(this, SIGNAL(fileChanged(bool)), this, SLOT(setCooldown()));
	QTimer::singleShot(interval_, this, SLOT(clearCooldown()));
	filename_.refresh();
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
	filename_.refresh();
	bool exists = filename_.exists();
	if (!exists) {
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
		Q_EMIT FileMonitor::fileChanged(exists);
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
