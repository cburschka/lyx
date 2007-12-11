// -*- C++ -*-
/**
 * \file SystemcallPrivate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_SUPPORT_SYSTEMCALLPRIVATE_H
#define LYX_SUPPORT_SYSTEMCALLPRIVATE_H

#include <QObject>
#include <QProcess>

#include <string>


namespace lyx {
namespace support {

class ProgressInterface;

class SystemcallPrivate : public QObject
{
	Q_OBJECT

public:
	SystemcallPrivate();

	// When waitForFinished == true :   returns the exit code of the process
	// When waitForFinished == false:   returns 0 if the process could be started
	int start(const std::string & cmd, bool waitForFinished);

public Q_SLOTS:
	void newProcessOutput();
	void processStarted();
	void processError(QProcess::ProcessError);
	void processFinished(int, QProcess::ExitStatus);
	
private:
	QProcess process;
};


} // namespace support
} // namespace lyx

#endif // LYX_SUPPORT_SYSTEMCALLPRIVATE_H
