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

class QProcess;

namespace lyx {
namespace support {

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

#endif // SYSTEMCALLPRIVATE_H
