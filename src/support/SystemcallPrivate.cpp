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


int SystemcallPrivate::start(const std::string& cmd, bool waitForFinished)
{
	ProgressInterface* progress = Systemcall::progress();
	if (progress) {
		progress->clearMessages();
		progress->appendMessage(qt_("Starting LaTex with command "));
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
		progress->appendMessage(qt_("LaTex started\n"));
	}
}


void SystemcallPrivate::processError(QProcess::ProcessError)
{
	ProgressInterface* progress = Systemcall::progress();
	if (progress) {
		progress->appendMessage(qt_("LaTex error\n"));
	}
}


void SystemcallPrivate::processFinished(int, QProcess::ExitStatus)
{
	deleteLater();
}


} // namespace support
} // namespace lyx

#include "SystemcallPrivate_moc.cpp"
