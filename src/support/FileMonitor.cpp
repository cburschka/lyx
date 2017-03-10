/**
 * \file FileMonitor.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
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
	// This thread-safe because QFileSystemWatcher is thread-safe.
	static FileSystemWatcher f;
	return f;
}


FileSystemWatcher::FileSystemWatcher()
	: qwatcher_(make_unique<QFileSystemWatcher>())
{}


//static
FileMonitorPtr FileSystemWatcher::monitor(FileName const & file_with_path)
{
	FileSystemWatcher & f = instance();
	string const filename = file_with_path.absFileName();
	weak_ptr<FileMonitorGuard> & wptr = f.store_[filename];
	if (shared_ptr<FileMonitorGuard> mon = wptr.lock())
		return make_unique<FileMonitor>(mon);
	auto mon = make_shared<FileMonitorGuard>(filename, f.qwatcher_.get());
	wptr = mon;
	return make_unique<FileMonitor>(mon);
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
	: filename_(filename), qwatcher_(qwatcher)
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


void FileMonitorGuard::refresh(bool new_file)
{
	QString const qfilename = toqstr(filename_);
	if(!qwatcher_->files().contains(qfilename)) {
		bool exists = QFile(qfilename).exists();
#if (QT_VERSION >= 0x050000)
		if (!exists || !qwatcher_->addPath(qfilename)) {
			if (exists)
				LYXERR(Debug::FILES,
				       "Could not add path to QFileSystemWatcher: "
				       << filename_);
			QTimer::singleShot(1000, this, [=](){
					refresh(new_file || !exists);
				});
#else
		auto add_path = [&]() {
			qwatcher_->addPath(qfilename);
			return qwatcher_->files().contains(qfilename);
		};
		if (!exists || !add_path()) {
			if (exists)
				LYXERR(Debug::FILES,
				       "Could not add path to QFileSystemWatcher: "
				       << filename_);
			if (new_file || !exists)
				QTimer::singleShot(1000, this, SLOT(refreshTrue()));
			else
				QTimer::singleShot(1000, this, SLOT(refreshFalse()));
#endif
		} else if (exists && new_file)
			Q_EMIT fileChanged();
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
	connectToFileMonitorGuard();
	refresh();
}


void FileMonitor::connectToFileMonitorGuard()
{
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


FileMonitorBlockerGuard::FileMonitorBlockerGuard(FileMonitor * parent)
	: QObject(parent), parent_(parent), delay_(0)
{
	QObject::disconnect(parent_->monitor_.get(), SIGNAL(fileChanged()),
	                    parent_, SLOT(changed()));
}


void FileMonitorBlockerGuard::setDelay(int delay)
{
	delay_ = max(delay_, delay);
}


FileMonitorBlockerGuard::~FileMonitorBlockerGuard()
{
	// closures can only copy local copies
	FileMonitor * parent = parent_;
	// parent is also our QObject::parent() so we are deleted before parent.
	// Even if delay_ is 0, the QTimer is necessary. Indeed, the notifications
	// from QFileSystemWatcher that we meant to ignore are not going to be
	// treated immediately, so we must yield to give us the opportunity to
	// ignore them.
	QTimer::singleShot(delay_, parent, SLOT(connectToFileMonitorGuard()));
}

} // namespace support
} // namespace lyx

#include "moc_FileMonitor.cpp"
