// -*- C++ -*-
/**
 * \file SystemcallPrivate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SYSTEMCALLPRIVATE_H
#define SYSTEMCALLPRIVATE_H

#include <QObject>
#include <QProcess>

#include <string>

namespace lyx {
namespace support {

class Systemcall;

/**
 * Outputs to the console terminal the line buffered standard output and
 * error of a spawned process when there is a controlling terminal and 
 * stdout/stderr have not been redirected.
 */
class SystemcallPrivate : public QObject
{
	Q_OBJECT

public:
	SystemcallPrivate(std::string const & infile, std::string const & outfile,
	                  std::string const & errfile);
	~SystemcallPrivate();

	enum State {
		Starting,
		Running,
		Finished,
		Error
	};
	State state;

	bool waitWhile(State, bool processEvents, int timeout = -1);
	void startProcess(QString const & cmd, std::string const & path,
	                  std::string const & lpath, bool detach);
	
	int exitCode();

	QString errorMessage() const;
	QString exitStatusMessage() const;

	QProcess* releaseProcess();
	
	static void killProcess(QProcess * p);


public Q_SLOTS:
	void stdOut();
	void stdErr();
	void processError(QProcess::ProcessError);
	void processStarted();
	void processFinished(int, QProcess::ExitStatus status);


private:
	/// Pointer to the process to monitor.
	QProcess * process_;

	/// Index to the standard output buffer.
	size_t out_index_;
	/// Index to the standard error buffer.
	size_t err_index_;
	///
	std::string in_file_;
	///
	std::string out_file_;
	///
	std::string err_file_;

	/// Size of buffers.
	static size_t const buffer_size_ = 200;
	/// Standard output buffer.
	char out_data_[buffer_size_];
	/// Standard error buffer.
	char err_data_[buffer_size_];

	QString cmd_;
	bool process_events_;

	void waitAndProcessEvents();
	void processEvents();
	void killProcess();	

};

} // namespace support
} // namespace lyx

#endif // SYSTEMCALLPRIVATE_H
