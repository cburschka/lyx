/**
 * \file Systemcall.cpp
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

#include <config.h>

#include "support/debug.h"
#include "support/qstring_helpers.h"
#include "support/Systemcall.h"
#include "support/os.h"

#include <cstdlib>
#include <iostream>

#include <QProcess>

#define USE_QPROCESS

using namespace std;

namespace lyx {
namespace support {

static void killProcess(QProcess * p)
{
	p->closeReadChannel(QProcess::StandardOutput);
	p->closeReadChannel(QProcess::StandardError);
	p->close();
	delete p;
}


// Reuse of instance
int Systemcall::startscript(Starttype how, string const & what)
{
#ifndef USE_QPROCESS
	string command = what;

	if (how == DontWait) {
		switch (os::shell()) {
		case os::UNIX:
			command += " &";
			break;
		case os::CMD_EXE:
			command = "start /min " + command;
			break;
		}
	}

	return ::system(command.c_str());
#else
	QString cmd = QString::fromLocal8Bit(what.c_str());
	QProcess * process = new QProcess;
	if (os::terminal_output()) {
		// Qt won't start the process if we redirect stdout and
		// stderr this way, without running in a terminal.
		process->setStandardOutputFile(toqstr(os::stdoutdev()));
		process->setStandardErrorFile(toqstr(os::stderrdev()));
	}
	process->start(cmd);
	if (!process->waitForStarted(3000)) {
		LYXERR0("Qprocess " << cmd << " did not start!");
		LYXERR0("error " << process->error());
		LYXERR0("state " << process->state());
		LYXERR0("status " << process->exitStatus());
		return 10;
	}
	if (how == DontWait)
		return 0;

	if (!process->waitForFinished(180000)) {
		LYXERR0("Qprocess " << cmd << " did not finished!");
		LYXERR0("error " << process->error());
		LYXERR0("state " << process->state());
		LYXERR0("status " << process->exitStatus());
		return 20;
	}
	int const exit_code = process->exitCode();
	if (exit_code) {
		LYXERR0("Qprocess " << cmd << " finished!");
		LYXERR0("exitCode " << process->exitCode());
		LYXERR0("error " << process->error());
		LYXERR0("state " << process->state());
		LYXERR0("status " << process->exitStatus());
	}
	if (!os::terminal_output()) {
	    // The output may have been redirected. But even if we are not
	    // running in a terminal, the output could go to some log file,
	    // for example ~/.xsession-errors on *nix.
	    cout << fromqstr(QString::fromLocal8Bit(process->readAllStandardOutput().data())) << endl;
	    cerr << fromqstr(QString::fromLocal8Bit(process->readAllStandardError().data())) << endl;
	}
	killProcess(process);
	return exit_code;
#endif
}

} // namespace support
} // namespace lyx
