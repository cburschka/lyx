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
	p->disconnect();
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
	ConOut console(process);
	if (!outfile.empty()) {
		// Check whether we have to simply throw away the output.
		if (outfile != os::nulldev())
			process->setStandardOutputFile(toqstr(outfile));
	} else if (os::is_terminal(os::STDOUT))
		console.showout();
	if (os::is_terminal(os::STDERR))
		console.showerr();

	process->start(cmd);
	if (!process->waitForStarted(3000)) {
		LYXERR0("Qprocess " << cmd << " did not start!");
		LYXERR0("error " << process->error());
		LYXERR0("state " << process->state());
		LYXERR0("status " << process->exitStatus());
		return 10;
	}
	if (how == DontWait) {
		// TODO delete process later
		return 0;
	}

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


ConOut::ConOut(QProcess * proc) : proc_(proc), outindex_(0), errindex_(0),
				  showout_(false), showerr_(false)
{
	connect(proc, SIGNAL(readyReadStandardOutput()), SLOT(stdOut()));
	connect(proc, SIGNAL(readyReadStandardError()), SLOT(stdErr()));
}


ConOut::~ConOut()
{
	if (outindex_) {
		outdata_[outindex_] = '\0';
		outindex_ = 0;
		cout << outdata_;
	}
	cout.flush();
	if (errindex_) {
		errdata_[errindex_] = '\0';
		errindex_ = 0;
		cerr << errdata_;
	}
	cerr.flush();
}


void ConOut::stdOut()
{
	if (showout_) {
		char c;
		proc_->setReadChannel(QProcess::StandardOutput);
		while (proc_->getChar(&c)) {
			outdata_[outindex_++] = c;
			if (c == '\n' || outindex_ + 1 == bufsize_) {
				outdata_[outindex_] = '\0';
				outindex_ = 0;
				cout << outdata_;
			}
		}
	}
}


void ConOut::stdErr()
{
	if (showerr_) {
		char c;
		proc_->setReadChannel(QProcess::StandardError);
		while (proc_->getChar(&c)) {
			errdata_[errindex_++] = c;
			if (c == '\n' || errindex_ + 1 == bufsize_) {
				errdata_[errindex_] = '\0';
				errindex_ = 0;
				cerr << errdata_;
			}
		}
	}
}

#include "moc_Systemcall.cpp"
#endif

} // namespace support
} // namespace lyx
