// -*- C++ -*-
/**
 * \file Systemcall.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 *
 * Interface cleaned up by
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SYSTEMCALL_H
#define SYSTEMCALL_H

#include <string>
#include <QObject>

class QProcess;

namespace lyx {
namespace support {

/**
 * An instance of Class Systemcall represents a single child process.
 *
 * Class Systemcall uses system() to launch the child process.
 * The user can choose to wait or not wait for the process to complete, but no
 * callback is invoked upon completion of the child.
 *
 * The child process is not killed when the Systemcall instance goes out of
 * scope.
 */
class Systemcall {
public:
	/// whether to wait for completion
	enum Starttype {
		Wait, //< wait for completion before returning from startscript()
		DontWait //< don't wait for completion
	};

	/** Start child process.
	 *  The string "what" contains a commandline with arguments separated
	 *  by spaces.
	 */
	int startscript(Starttype how, std::string const & what);
};


/**
 * Outputs to the console terminal the line buffered standard output and
 * error of a spawned process when there is a controlling terminal and 
 * stdout/stderr have not been redirected.
 */
class ConOut : public QObject
{
	Q_OBJECT
public:
	ConOut(QProcess * proc);
	~ConOut();

	/// Should the standard output be displayed?
	void showout() { showout_ = true; }

	/// Should the standard error be displayed?
	void showerr() { showerr_ = true; }

private:
	/// Pointer to the process to monitor.
	QProcess * proc_;
	/// Index to the standard output buffer.
	size_t outindex_;
	/// Index to the standard error buffer.
	size_t errindex_;
	/// Size of buffers.
	static size_t const bufsize_ = 200;
	/// Standard output buffer.
	char outdata_[bufsize_];
	/// Standard error buffer.
	char errdata_[bufsize_];
	/// 
	bool showout_;
	/// 
	bool showerr_;

public Q_SLOTS:
	void stdOut();
	void stdErr();
};

} // namespace support
} // namespace lyx

#endif // SYSTEMCALL_H
