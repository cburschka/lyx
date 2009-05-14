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
#include "support/lstrings.h"
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
#ifndef USE_QPROCESS
int Systemcall::startscript(Starttype how, string const & what)
{
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
}

#else

namespace {

string const parsecmd(string const & cmd, string & outfile)
{
	bool inquote = false;
	bool escaped = false;

	for (size_t i = 0; i < cmd.length(); ++i) {
		char c = cmd[i];
		if (c == '"' && !escaped)
			inquote = !inquote;
		else if (c == '\\' && !escaped)
			escaped = !escaped;
		else if (c == '>' && !(inquote || escaped)) {
			outfile = trim(cmd.substr(i + 1), " \"");
			return trim(cmd.substr(0, i));
		} else
			escaped = false;
	}
	outfile.erase();
	return cmd;

}

} // namespace anon


int Systemcall::startscript(Starttype how, string const & what)
{
	string outfile;
	QString cmd = toqstr(parsecmd(what, outfile));
	QProcess * process = new QProcess;

	// Qt won't start the process if we redirect stdout/stderr in
	// this way and they are not connected to a terminal (maybe
	// because we were launched from some desktop GUI).
	if (!outfile.empty()) {
		// Check whether we have to simply throw away the output.
		if (outfile != os::nulldev())
			process->setStandardOutputFile(toqstr(outfile));
	} else if (os::is_terminal(os::STDOUT))
		process->setStandardOutputFile(toqstr(os::stdoutdev()));
	if (os::is_terminal(os::STDERR))
		process->setStandardErrorFile(toqstr(os::stderrdev()));

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

	// If the output has been redirected, we write it all at once.
	// Even if we are not running in a terminal, the output could go
	// to some log file, for example ~/.xsession-errors on *nix.
	if (!os::is_terminal(os::STDOUT) && outfile.empty())
		cout << fromqstr(QString::fromLocal8Bit(
			    process->readAllStandardOutput().data()));
	if (!os::is_terminal(os::STDERR))
		cerr << fromqstr(QString::fromLocal8Bit(
			    process->readAllStandardError().data()));

	killProcess(process);
	return exit_code;
}
#endif

} // namespace support
} // namespace lyx
