// -*- C++ -*-
/**
 * \file FileMonitor.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 *
 * FileMonitor monitors a file and informs a listener when that file has
 * changed.
 */

#ifndef FILEMONITOR_H
#define FILEMONITOR_H

#include <memory>

#include <QFileSystemWatcher>
#include <QObject>

#include <boost/signals2.hpp>


namespace lyx {
namespace support {

class FileName;

///
///  FileMonitor, a file monitor based on QFileSystemWatcher
///

class FileMonitor;
class FileMonitorGuard;
typedef std::unique_ptr<FileMonitor> FileMonitorPtr;

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
/// Block notifications for the duration of a scope:
///   {
///       FileMonitorBlocker block = monitor.block();
///       ...
///   }
///
/// Reset connections:
///   monitor.disconnect();
///   or the disconnect method of the connection object for the boost signal.
///
class FileSystemWatcher
{
public:
	// as described above
	static FileMonitorPtr monitor(FileName const & file_with_path);
	// Output whether the paths tracked by qwatcher_ and the active
	// FileMonitorGuards are in correspondence.
	static void debug();
private:
	FileSystemWatcher();
	// A global instance is created automatically on first call to monitor
	static FileSystemWatcher & instance();
	// Caches the monitor guards but allow them to be destroyed
	std::map<std::string, std::weak_ptr<FileMonitorGuard>> store_;
	// This class is a wrapper for QFileSystemWatcher
	std::unique_ptr<QFileSystemWatcher> const qwatcher_;
};


// Must be unique per path
// Ends the watch when deleted
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
	/// Make sure it is being monitored, after e.g. a deletion. See
	/// <https://bugreports.qt.io/browse/QTBUG-46483>. This is called
	/// automatically.
	/// \param new_file  If true, emit fileChanged if the file exists and was
	/// successfully added.
	void refresh(bool new_file = false);

Q_SIGNALS:
	/// Connect to this to be notified when the file changes
	void fileChanged() const;

private Q_SLOTS:
	/// Receive notifications from the QFileSystemWatcher
	void notifyChange(QString const & path);

	/// nonsense introduced for when QT_VERSION < 0x050000, cannot be placed
	/// between #ifdef
	void refreshTrue() { refresh(true); }
	/// nonsense introduced for when QT_VERSION < 0x050000, cannot be placed
	/// between #ifdef
	void refreshFalse() { refresh(false); }


private:
	std::string const filename_;
	QFileSystemWatcher * qwatcher_;
};


class FileMonitorBlockerGuard : public QObject
{
	Q_OBJECT
	FileMonitor * parent_;
	int delay_;

public:
	FileMonitorBlockerGuard(FileMonitor * parent);
	~FileMonitorBlockerGuard();
	void setDelay(int delay);
};


typedef std::shared_ptr<FileMonitorBlockerGuard> FileMonitorBlocker;


/// Main class
class FileMonitor : public QObject
{
	Q_OBJECT
	friend class FileMonitorBlockerGuard;

public:
	FileMonitor(std::shared_ptr<FileMonitorGuard> monitor);

	typedef boost::signals2::signal<void()> sig;
	/// Connect and you'll be informed when the file has changed.
	boost::signals2::connection connect(sig::slot_type const &);
	/// disconnect all slots connected to the boost signal fileChanged_ or to
	/// the qt signal fileChanged()
	void disconnect();
	/// absolute path being tracked
	std::string const & filename() { return monitor_->filename(); }
	/// Creates a guard that blocks notifications. Copyable. Notifications from
	/// this monitor are blocked as long as there are copies around.
	/// \param delay is the amount waited in ms after expiration of the guard
	/// before reconnecting. This delay thing is to deal with asynchronous
	/// notifications in a not so elegant fashion. But it can also be used to
	/// slow down incoming events.
	FileMonitorBlocker block(int delay = 0);
	/// Make sure the good file is being monitored, after e.g. a move or a
	/// deletion. See <https://bugreports.qt.io/browse/QTBUG-46483>. This is
	/// called automatically.
	void refresh() { return monitor_->refresh(); }

Q_SIGNALS:
	/// Connect to this to be notified when the file changes
	void fileChanged() const;

private Q_SLOTS:
	/// Receive notifications from the FileMonitorGuard
	void changed();
	///
	void connectToFileMonitorGuard();

private:
	// boost signal
	sig fileChanged_;
	// the unique watch for our file
	std::shared_ptr<FileMonitorGuard> const monitor_;
	//
	std::weak_ptr<FileMonitorBlockerGuard> blocker_;
};



} // namespace support
} // namespace lyx

#endif // FILEMONITOR_H
