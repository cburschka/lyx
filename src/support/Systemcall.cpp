/**
 * \file Systemcall.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Angus Leeming
 * \author Enrico Forestieri
 * \author Peter Kuemmel
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/debug.h"
#include "support/lstrings.h"
#include "support/qstring_helpers.h"
#include "support/Systemcall.h"
#include "support/SystemcallPrivate.h"
#include "support/os.h"


#include <cstdlib>
#include <iostream>

#include <QProcess>
#include <QTime>
#include <QThread>
#include <QCoreApplication>
#include <QDebug>

#define USE_QPROCESS


struct Sleep : QThread
{
	static void millisec(unsigned long ms) 
	{
		QThread::usleep(ms * 1000);
	}
};



using namespace std;

namespace lyx {
namespace support {



// Reuse of instance
#ifndef USE_QPROCESS
int Systemcall::startscript(Starttype how, string const & what,
							bool /*process_events*/)
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

/*
 * This is a parser that (mostly) mimics the behavior of a posix shell but
 * its output is tailored for being processed by QProcess.
 *
 * The escape character is the backslash.
 * A backslash that is not quoted preserves the literal value of the following
 * character, with the exception of a double-quote '"'. If a double-quote
 * follows a backslash, it will be replaced by three consecutive double-quotes
 * (this is how the QProcess parser recognizes a '"' as a simple character
 * instead of a quoting character). Thus, for example:
 *     \\  ->  \
 *     \a  ->  a
 *     \"  ->  """
 *
 * Single-quotes.
 * Characters enclosed in single-quotes ('') have their literal value preserved.
 * A single-quote cannot occur within single-quotes. Indeed, a backslash cannot
 * be used to escape a single-quote in a single-quoted string. In other words,
 * anything enclosed in single-quotes is passed as is, but the single-quotes
 * themselves are eliminated. Thus, for example:
 *    '\'    ->  \
 *    '\\'   ->  \\
 *    '\a'   ->  \a
 *    'a\"b' ->  a\"b
 *
 * Double-quotes.
 * Characters enclosed in double-quotes ("") have their literal value preserved,
 * with the exception of the backslash. The backslash retains its special
 * meaning as an escape character only when followed by a double-quote.
 * Contrarily to the behavior of a posix shell, the double-quotes themselves
 * are *not* eliminated. Thus, for example:
 *    "\\"   ->  "\\"
 *    "\a"   ->  "\a"
 *    "a\"b" ->  "a"""b"
 */
string const parsecmd(string const & inputcmd, string & outfile)
{
	bool in_single_quote = false;
	bool in_double_quote = false;
	bool escaped = false;
	string cmd;

	for (size_t i = 0; i < inputcmd.length(); ++i) {
		char c = inputcmd[i];
		if (c == '\'') {
			if (in_double_quote || escaped) {
				if (in_double_quote && escaped)
					cmd += '\\';
				cmd += c;
			} else
				in_single_quote = !in_single_quote;
			escaped = false;
			continue;
		}
		if (in_single_quote) {
			cmd += c;
			continue;
		}
		if (c == '"') {
			if (escaped) {
				cmd += "\"\"\"";
				escaped = false;
			} else {
				cmd += c;
				in_double_quote = !in_double_quote;
			}
		} else if (c == '\\' && !escaped) {
			escaped = !escaped;
		} else if (c == '>' && !(in_double_quote || escaped)) {
			outfile = trim(inputcmd.substr(i + 1), " \"");
			return trim(cmd);
		} else {
			if (escaped && in_double_quote)
				cmd += '\\';
			cmd += c;
			escaped = false;
		}
	}
	outfile.erase();
	return cmd;
}

} // namespace anon



int Systemcall::startscript(Starttype how, string const & what, bool process_events)
{
	string outfile;
	QString cmd = toqstr(parsecmd(what, outfile));

	SystemcallPrivate d(outfile);


	d.startProcess(cmd);
	if (!d.waitWhile(SystemcallPrivate::Starting, process_events, -1)) {
		LYXERR0("Systemcall: '" << cmd << "' did not start!");
		LYXERR0("error " << d.errorMessage());
		return 10;
	}

	if (how == DontWait) {
		QProcess* released = d.releaseProcess();
		(void) released; // TODO who deletes it?
		return 0;
	}

	if (!d.waitWhile(SystemcallPrivate::Running, process_events, 180000)) {
		LYXERR0("Systemcall: '" << cmd << "' did not finished!");
		LYXERR0("error " << d.errorMessage());
		LYXERR0("status " << d.exitStatusMessage());
		return 20;
	}

	int const exit_code = d.exitCode();
	if (exit_code) {
		LYXERR0("Systemcall: '" << cmd << "' finished with exit code " << exit_code);
	}

	return exit_code;
}


SystemcallPrivate::SystemcallPrivate(const std::string& of) : 
                                proc_(new QProcess), outindex_(0), errindex_(0),
                                outfile(of), showout_(false), showerr_(false), process_events(false)
{
	if (!outfile.empty()) {
		// Check whether we have to simply throw away the output.
		if (outfile != os::nulldev())
			proc_->setStandardOutputFile(toqstr(outfile));
	} else if (os::is_terminal(os::STDOUT))
		showout();
	if (os::is_terminal(os::STDERR))
		showerr();

	connect(proc_, SIGNAL(readyReadStandardOutput()), SLOT(stdOut()));
	connect(proc_, SIGNAL(readyReadStandardError()), SLOT(stdErr()));
	connect(proc_, SIGNAL(error(QProcess::ProcessError)), SLOT(processError(QProcess::ProcessError)));
	connect(proc_, SIGNAL(started()), this, SLOT(processStarted()));
	connect(proc_, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(processFinished(int, QProcess::ExitStatus)));
}



void SystemcallPrivate::startProcess(const QString& cmd)
{
	if (proc_) {
		state = SystemcallPrivate::Starting;
		proc_->start(cmd);
	}
}


void SystemcallPrivate::processEvents()
{
	if(process_events) {
		QCoreApplication::processEvents(QEventLoop::AllEvents);
	}
}


void SystemcallPrivate::waitAndProcessEvents()
{
	Sleep::millisec(100);
	processEvents();
}


bool SystemcallPrivate::waitWhile(State waitwhile, bool proc_events, int timeout)
{
	if (!proc_)
		return false;

	process_events = proc_events;

	// Block GUI while waiting,
	// relay on QProcess' wait functions
	if (!process_events) {
		if (waitwhile == Starting)
			return proc_->waitForStarted(timeout);
		if (waitwhile == Running)
			return proc_->waitForFinished(timeout);
		return false;
	}

	// process events while waiting, no timeout
	if (timeout == -1) {
		while (state == waitwhile && state != Error) {
			waitAndProcessEvents();
		}
		return state != Error;
	} 

	// process events while waiting whith timeout
	QTime timer;
	timer.start();
	while (state == waitwhile && state != Error && timer.elapsed() < timeout) {
		waitAndProcessEvents();
	}
	return (state != Error) && (timer.elapsed() < timeout);
}


SystemcallPrivate::~SystemcallPrivate()
{
	flush();

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

	killProcess();
}


void SystemcallPrivate::flush()
{
	if (proc_) {
		// If the output has been redirected, we write it all at once.
		// Even if we are not running in a terminal, the output could go
		// to some log file, for example ~/.xsession-errors on *nix.
		if (!os::is_terminal(os::STDOUT) && outfile.empty())
			cout << fromqstr(QString::fromLocal8Bit(
						proc_->readAllStandardOutput().data()));
		if (!os::is_terminal(os::STDERR))
			cerr << fromqstr(QString::fromLocal8Bit(
						proc_->readAllStandardError().data()));
	}
}


void SystemcallPrivate::stdOut()
{
	if (proc_ && showout_) {
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
	processEvents();
}


void SystemcallPrivate::stdErr()
{
	if (proc_ && showerr_) {
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
	processEvents();
}


void SystemcallPrivate::processStarted()
{
	state = Running;
	// why do we get two started signals?
	//disconnect(proc_, SIGNAL(started()), this, SLOT(processStarted()));
}


void SystemcallPrivate::processFinished(int, QProcess::ExitStatus)
{
	state = Finished;
}


void SystemcallPrivate::processError(QProcess::ProcessError)
{
	state = Error;
}


QString SystemcallPrivate::errorMessage() const 
{
	if (!proc_)
		return "No QProcess available";

	QString message;
	switch (proc_->error()) {
		case QProcess::FailedToStart:
			message = "The process failed to start. Either the invoked program is missing, "
				      "or you may have insufficient permissions to invoke the program.";
			break;
		case QProcess::Crashed:
			message = "The process crashed some time after starting successfully.";
			break;
		case QProcess::Timedout:
			message = "The process timed out. It might be restarted automatically.";
			break;
		case QProcess::WriteError:
			message = "An error occurred when attempting to write to the process-> For example, "
				      "the process may not be running, or it may have closed its input channel.";
			break;
		case QProcess::ReadError:
			message = "An error occurred when attempting to read from the process-> For example, "
				      "the process may not be running.";
			break;
		case QProcess::UnknownError:
		default:
			message = "An unknown error occured.";
			break;
	}
	return message;
}


QString SystemcallPrivate::exitStatusMessage() const
{
	if (!proc_)
		return "No QProcess available";

	QString message;
	switch (proc_->exitStatus()) {
		case QProcess::NormalExit:
			message = "The process exited normally.";
			break;
		case QProcess::CrashExit:
			message = "The process crashed.";
			break;
		default:
			message = "Unknown exit state.";
			break;
	}
	return message;
}


int SystemcallPrivate::exitCode()
{
	if (!proc_)
		return -1;

	return proc_->exitCode();
}


QProcess* SystemcallPrivate::releaseProcess()
{
	QProcess* released = proc_;
	proc_ = 0;
	return released;
}


void SystemcallPrivate::killProcess()
{
	killProcess(proc_);
}


void SystemcallPrivate::killProcess(QProcess * p)
{
	if (p) {
		p->disconnect();
		p->closeReadChannel(QProcess::StandardOutput);
		p->closeReadChannel(QProcess::StandardError);
		p->close();
		delete p;
	}
}



#include "moc_SystemcallPrivate.cpp"
#endif

} // namespace support
} // namespace lyx
