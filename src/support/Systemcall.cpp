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
	SystemcallPrivate d(outfile);

	bool process_events = true;

	d.startProcess(cmd);
	if (!d.waitWhile(SystemcallPrivate::Starting, process_events, 3000)) {
		LYXERR0("QProcess " << cmd << " did not start!");
		LYXERR0("error " << d.errorMessage());
		return 10;
	}

	if (how == DontWait) {
		QProcess* released = d.releaseProcess();
		(void) released; // TODO who deletes it?
		return 0;
	}

	if (!d.waitWhile(SystemcallPrivate::Running, process_events, 180000)) {
		LYXERR0("QProcess " << cmd << " did not finished!");
		LYXERR0("error " << d.errorMessage());
		LYXERR0("status " << d.exitStatusMessage());
		return 20;
	}

	int const exit_code = d.exitCode();
	if (exit_code) {
		LYXERR0("QProcess " << cmd << " finished!");
		LYXERR0("error " << exit_code << ": " << d.errorMessage()); 
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
		//static int count = 0; qDebug() << count++ << ": waitAndProcessEvents";
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


void SystemcallPrivate::processFinished(int, QProcess::ExitStatus status)
{
	state = Finished;
}


void SystemcallPrivate::processError(QProcess::ProcessError err)
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
