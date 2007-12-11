// -*- C++ -*-
/**
 * \file SystemcallPrivate.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "SystemcallPrivate.h"

#include "Systemcall.h"
#include "ProgressInterface.h"

#include "gettext.h"
#include "qstring_helpers.h"


namespace lyx {
namespace support {


// TODO should we move qt_ to qstring_helpers?
static
QString const qt_(char const * str)
{
	return toqstr(_(str));
}



SystemcallPrivate::SystemcallPrivate() 
{ 
	ProgressInterface* progress = Systemcall::progress();
	if (progress) {
		connect(&process, SIGNAL(readyReadStandardOutput()), this, SLOT(newProcessOutput()));
		connect(&process, SIGNAL(started()), this, SLOT(processStarted()));
		connect(&process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
		connect(&process, SIGNAL(finished(int, QProcess::ExitStatus)), 
				this, SLOT(processFinished(int, QProcess::ExitStatus)));				
	}
}


int SystemcallPrivate::start(const std::string & cmd, bool waitForFinished)
{
	ProgressInterface* progress = Systemcall::progress();
	if (progress) {
		progress->clearMessages();
		progress->appendMessage(qt_("Starting process with command "));
		progress->appendMessage(cmd.c_str());
	}

	process.setReadChannel(QProcess::StandardOutput);
	process.start(cmd.c_str(), QStringList(), QIODevice::ReadOnly);
	// wait some seconds until the process has started
	process.waitForStarted(10 * 1000);
	if (waitForFinished) {
		// with waitForFinished(-1); we only get one signal per run
		while (process.state() == QProcess::Running)
			process.waitForFinished(500);
		return process.exitCode();
	}
	if (process.state() != QProcess::Running) {
		process.kill();
		// TODO this needs more testing
		deleteLater();
		return -1;
	}
	return 0;
}


void SystemcallPrivate::newProcessOutput()
{
	ProgressInterface* progress = Systemcall::progress();
	if (progress) {
		const QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
		progress->appendMessage(output);
	}
}


void SystemcallPrivate::processStarted()
{
	ProgressInterface* progress = Systemcall::progress();
	if (progress) {
		progress->appendMessage(qt_("Process started\n"));
	}
}


void SystemcallPrivate::processError(QProcess::ProcessError err)
{
	ProgressInterface* progress = Systemcall::progress();
	if (progress) {
		QString message;
		switch (err) {
			case QProcess::FailedToStart:
				message = qt_("The process failed to start. Either the invoked program is missing, "
					      "or you may have insufficient permissions to invoke the program.");
				break;
			case QProcess::Crashed:
				message = qt_("The process crashed some time after starting successfully.");
				break;
			case QProcess::Timedout:
				message = qt_("The process timed out. It might be restarted automatically.");
				break;
			case QProcess::WriteError:
				message = qt_("An error occurred when attempting to write to the process. For example, "
					      "the process may not be running, or it may have closed its input channel.");
				break;
			case QProcess::ReadError:
				message = qt_("An error occurred when attempting to read from the process. For example, "
					      "the process may not be running.");
				break;
			case QProcess::UnknownError:
			default:
				message = qt_("An unknown error occured.");
				break;
		}
		progress->appendMessage(qt_("The process failed: ") + message + '\n');
	}
}


void SystemcallPrivate::processFinished(int, QProcess::ExitStatus status)
{
	ProgressInterface* progress = Systemcall::progress();
	if (progress) {
		QString message;
		switch (status) {
			case QProcess::NormalExit:
				message = qt_("The process exited normally.");
				break;
			case QProcess::CrashExit:
				message = qt_("The process crashed.");
				break;
			default:
				message = qt_("Unknown exit state.");
				break;
		}
		progress->appendMessage(qt_("Process finished: ") + message + '\n');
	}
	deleteLater();
}


} // namespace support
} // namespace lyx

#include "SystemcallPrivate_moc.cpp"
