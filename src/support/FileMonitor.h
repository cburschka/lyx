// -*- C++ -*-
/**
 * \file FileMonitor.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 *
 * FileMonitor monitors a file and informs a listener when that file has
 * changed.
 */

#ifndef FILEMONITOR_H
#define FILEMONITOR_H

#include "support/FileName.h"
#include "support/signals.h"

#include <memory>
#include <map>

#include <QFileSystemWatcher>
#include <QObject>
#include <QPointer>


namespace lyx {
namespace support {

///
///  FileMonitor, a file monitor based on QFileSystemWatcher
///

class FileMonitor;
class ActiveFileMonitor;
class FileMonitorGuard;
typedef std::unique_ptr<FileMonitor> FileMonitorPtr;
typedef std::unique_ptr<ActiveFileMonitor> ActiveFileMonitorPtr;

///
/// Watch a file:
///   FileMonitorPtr monitor = FileSystemWatcher::monitor(file_with_path);
///   monitor.connect(...); //(using boost::signals2), or:
///   connect(monitor, SIGNAL(fileChanged()),...); // (using Qt)
///
/// Remember that a unique_ptr is automatically deleted at the end of a scope if
/// it has not been moved, or when assigned. When that happens, the signal
/// object is deleted and therefore all the connections are closed. The file
/// ceases being tracked when all the monitors for a file have been deleted.
///
/// Stop watching:
///   * as determined statically by the scope, or
///   * dynamically, using:
///       monitor = nullptr;
///
/// Watch a different file:
///   monitor = FileSystemWatcher::monitor(file_with_path2);
///   monitor.connect(...);
/// (stops watching the first)
///
class FileSystemWatcher
{
public:
	/// as described above
	static FileMonitorPtr monitor(FileName const & filename);
	/// same but with an ActiveFileMonitor
	static ActiveFileMonitorPtr activeMonitor(FileName const & filename,
	                                          int interval = 10000);
	/// Output whether the paths tracked by qwatcher_ and the active
	/// FileMonitorGuards are in correspondence.
	static void debug();
private:
	FileSystemWatcher();
	/// A global instance is created automatically on first call
	static FileSystemWatcher & instance();
	///
	std::shared_ptr<FileMonitorGuard> getGuard(FileName const & filename);
	/// Caches the monitor guards but allow them to be destroyed
	std::map<std::string, std::weak_ptr<FileMonitorGuard>> store_;
	/// This class is a wrapper for QFileSystemWatcher
	std::unique_ptr<QFileSystemWatcher> const qwatcher_;
};


/// Must be unique per path
/// Ends the watch when deleted
class FileMonitorGuard : public QObject
{
	Q_OBJECT

public:
	/// Start the watch
	FileMonitorGuard(std::string const & filename,
	                 QFileSystemWatcher * qwatcher);
	/// End the watch
	~FileMonitorGuard();
	/// absolute path being tracked
	std::string const & filename() { return filename_; }

public Q_SLOTS:
	/// Make sure it is being monitored, after e.g. a deletion. See
	/// <https://bugreports.qt.io/browse/QTBUG-46483>. This is called
	/// automatically.
	void refresh(bool emit = true);

Q_SIGNALS:
	/// Connect to this to be notified when the file changes
	void fileChanged(bool exists) const;

private Q_SLOTS:
	/// Receive notifications from the QFileSystemWatcher
	void notifyChange(QString const & path);

private:
	std::string const filename_;
	QFileSystemWatcher * qwatcher_;
	/// for emitting fileChanged() when the file is created or deleted
	bool exists_;
};


/// Main class
class FileMonitor : public QObject
{
	Q_OBJECT

public:
	FileMonitor(std::shared_ptr<FileMonitorGuard> monitor);

	typedef signals2::signal<void(bool)> sig;
	typedef sig::slot_type slot;
	/// Connect and you'll be informed when the file has changed.
	signals2::connection connect(slot const &);
	/// absolute path being tracked
	std::string const & filename() { return monitor_->filename(); }
	/// Make sure the good file is being monitored, after e.g. a move or a
	/// deletion. See <https://bugreports.qt.io/browse/QTBUG-46483>. This is
	/// called automatically.
	void refresh() { monitor_->refresh(); }

Q_SIGNALS:
	/// Connect to this to be notified when the file changes
	void fileChanged(bool exists) const;

protected Q_SLOTS:
	/// Receive notifications from the FileMonitorGuard
	void changed(bool exists);
	///
	void connectToFileMonitorGuard();

private:
	/// boost signal
	sig fileChanged_;
	/// the unique watch for our file
	std::shared_ptr<FileMonitorGuard> const monitor_;
};


/// When a more active monitoring style is needed.
/// For instance because QFileSystemWatcher does not work for remote file
/// systems.
class ActiveFileMonitor : public FileMonitor
{
	Q_OBJECT
public:
	ActiveFileMonitor(std::shared_ptr<FileMonitorGuard> monitor,
	                  FileName const & filename, int interval);
	/// call checkModified asynchronously
	void checkModifiedAsync();

public Q_SLOTS:
	/// Check explicitly for a modification, but not more than once every
	/// interval ms.
	void checkModified();

private Q_SLOTS:
	void setCooldown() { cooldown_ = true; }
	void clearCooldown() { cooldown_ = false; }

private:
	FileName const filename_;
	///
	int const interval_;
	///
	time_t timestamp_;
	///
	unsigned long checksum_;
	///
	bool cooldown_;
};


} // namespace support
} // namespace lyx

#endif // FILEMONITOR_H
