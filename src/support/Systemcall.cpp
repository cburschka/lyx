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
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/qstring_helpers.h"
#include "support/Systemcall.h"
#include "support/SystemcallPrivate.h"
#include "support/os.h"
#include "support/ProgressInterface.h"

#include "LyXRC.h"

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


class ProgressDummy : public ProgressInterface 
{
public:
	ProgressDummy() {}

	void processStarted(QString const &) {}
	void processFinished(QString const &) {}
	void appendMessage(QString const &) {}
	void appendError(QString const &) {}
	void clearMessages() {}
	void lyxerrFlush() {}

	void lyxerrConnect() {}
	void lyxerrDisconnect() {}

	void warning(QString const &, QString const &) {}
	void toggleWarning(QString const &, QString const &, QString const &) {}
	void error(QString const &, QString const &, QString const &) {}
	void information(QString const &, QString const &) {}
	int prompt(docstring const &, docstring const &, int default_but, int,
		   docstring const &, docstring const &) { return default_but; }
};


static ProgressInterface * progress_instance = 0;

void ProgressInterface::setInstance(ProgressInterface* p)
{
	progress_instance = p;
}


ProgressInterface * ProgressInterface::instance()
{
	if (!progress_instance) {
		static ProgressDummy dummy;
		return &dummy;
	}
	return progress_instance;
}




// Reuse of instance
#ifndef USE_QPROCESS
int Systemcall::startscript(Starttype how, string const & what,
			    std::string const & path, bool /*process_events*/)
{
	string command = to_filesystem8bit(from_utf8(latexEnvCmdPrefix(path)))
		       + commandPrep(what);

	if (how == DontWait) {
		switch (os::shell()) {
		case os::UNIX:
			command += " &";
			break;
		case os::CMD_EXE:
			command = "start /min " + command;
			break;
		}
	} else if (os::shell() == os::CMD_EXE)
		command = subst(command, "cmd /d /c ", "");

	return ::system(command.c_str());
}

#else

namespace {

/*
 * This is a parser that (mostly) mimics the behavior of a posix shell as
 * regards quoting, but its output is tailored for being processed by QProcess.
 * Note that shell metacharacters are not parsed.
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
string const parsecmd(string const & incmd, string & infile, string & outfile,
                     string & errfile)
{
	bool in_single_quote = false;
	bool in_double_quote = false;
	bool escaped = false;
	string const python_call = "python -tt";
	vector<string> outcmd(4);
	size_t start = 0;

	if (prefixIs(incmd, python_call)) {
		outcmd[0] = os::python();
		start = python_call.length();
	}

	for (size_t i = start, o = 0; i < incmd.length(); ++i) {
		char c = incmd[i];
		if (c == '\'') {
			if (in_double_quote || escaped) {
				if (in_double_quote && escaped)
					outcmd[o] += '\\';
				outcmd[o] += c;
			} else
				in_single_quote = !in_single_quote;
			escaped = false;
			continue;
		}
		if (in_single_quote) {
			outcmd[o] += c;
			continue;
		}
		if (c == '"') {
			if (escaped) {
				// Don't triple double-quotes for redirection
				// files as these won't be parsed by QProcess
				outcmd[o] += string(o ? "\"" : "\"\"\"");
				escaped = false;
			} else {
				outcmd[o] += c;
				in_double_quote = !in_double_quote;
			}
		} else if (c == '\\' && !escaped) {
			escaped = !escaped;
		} else if (c == '>' && !(in_double_quote || escaped)) {
			if (suffixIs(outcmd[o], " 2")) {
				outcmd[o] = rtrim(outcmd[o], "2");
				o = 2;
			} else {
				if (suffixIs(outcmd[o], " 1"))
					outcmd[o] = rtrim(outcmd[o], "1");
				o = 1;
			}
		} else if (c == '<' && !(in_double_quote || escaped)) {
			o = 3;
		} else {
			if (escaped && in_double_quote)
				outcmd[o] += '\\';
			outcmd[o] += c;
			escaped = false;
		}
	}
	infile  = trim(outcmd[3], " \"");
	outfile = trim(outcmd[1], " \"");
	errfile = trim(outcmd[2], " \"");
	return trim(outcmd[0]);
}

} // namespace anon



int Systemcall::startscript(Starttype how, string const & what,
			    string const & path, bool process_events)
{
	string const what_ss = commandPrep(what);
	LYXERR(Debug::INFO,"Running: " << what_ss);

	string infile;
	string outfile;
	string errfile;
	QString const cmd = QString::fromLocal8Bit(
			parsecmd(what_ss, infile, outfile, errfile).c_str());

	SystemcallPrivate d(infile, outfile, errfile);

#ifdef Q_OS_WIN32
	// QProcess::startDetached cannot provide environment variables. When the
	// environment variables are set using the latexEnvCmdPrefix and the process
	// is started with QProcess::startDetached, a console window is shown every 
	// time a viewer is started. To avoid this, we fall back on Windows to the 
	// original implementation that creates a QProcess object.
	d.startProcess(cmd, path, false);
	if (!d.waitWhile(SystemcallPrivate::Starting, process_events, -1)) {
		LYXERR0("Systemcall: '" << cmd << "' did not start!");
		LYXERR0("error " << d.errorMessage());
		return 10;
	}
	if (how == DontWait) {
		d.releaseProcess();
		return 0;
	}
#else
	d.startProcess(cmd, path, how == DontWait);
	if (how == DontWait && d.state == SystemcallPrivate::Running)
		return 0;

	if (d.state == SystemcallPrivate::Error
			|| !d.waitWhile(SystemcallPrivate::Starting, process_events, -1)) {
		LYXERR0("Systemcall: '" << cmd << "' did not start!");
		LYXERR0("error " << d.errorMessage());
		return 10;
	}
#endif

	if (!d.waitWhile(SystemcallPrivate::Running, process_events,
			 os::timeout_min() * 60 * 1000)) {
		LYXERR0("Systemcall: '" << cmd << "' did not finish!");
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


SystemcallPrivate::SystemcallPrivate(std::string const & sf,
				     std::string const & of,
				     std::string const & ef) :
				process_(new QProcess), 
				out_index_(0),
				err_index_(0),
				in_file_(sf),
				out_file_(of), 
				err_file_(ef), 
				process_events_(false)
{
	if (!in_file_.empty())
		process_->setStandardInputFile(QString::fromLocal8Bit(in_file_.c_str()));
	if (!out_file_.empty()) {
		if (out_file_[0] == '&') {
			if (subst(out_file_, " ", "") == "&2"
			    && err_file_[0] != '&') {
				out_file_ = err_file_;
				process_->setProcessChannelMode(
						QProcess::MergedChannels);
			} else {
				if (err_file_[0] == '&') {
					// Leave alone things such as
					// "1>&2 2>&1". Should not be harmful,
					// but let's give anyway a warning.
					LYXERR0("Unsupported stdout/stderr redirect.");
					err_file_.erase();
				} else {
					LYXERR0("Ambiguous stdout redirect: "
						<< out_file_);
				}
				out_file_ = os::nulldev();
			}
		}
		// Check whether we have to set the output file.
		if (out_file_ != os::nulldev()) {
			process_->setStandardOutputFile(QString::fromLocal8Bit(
							out_file_.c_str()));
		}
	}
	if (!err_file_.empty()) {
		if (err_file_[0] == '&') {
			if (subst(err_file_, " ", "") == "&1"
			    && out_file_[0] != '&') {
				process_->setProcessChannelMode(
						QProcess::MergedChannels);
			} else {
				LYXERR0("Ambiguous stderr redirect: "
					<< err_file_);
			}
			// In MergedChannels mode stderr goes to stdout.
			err_file_ = os::nulldev();
		}
		// Check whether we have to set the error file.
		if (err_file_ != os::nulldev()) {
			process_->setStandardErrorFile(QString::fromLocal8Bit(
							err_file_.c_str()));
		}
	}

	connect(process_, SIGNAL(readyReadStandardOutput()), SLOT(stdOut()));
	connect(process_, SIGNAL(readyReadStandardError()), SLOT(stdErr()));
	connect(process_, SIGNAL(error(QProcess::ProcessError)), SLOT(processError(QProcess::ProcessError)));
	connect(process_, SIGNAL(started()), this, SLOT(processStarted()));
	connect(process_, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(processFinished(int, QProcess::ExitStatus)));
}


void SystemcallPrivate::startProcess(QString const & cmd, string const & path, bool detached)
{
	cmd_ = cmd;
	if (detached) {
		state = SystemcallPrivate::Running;
		if (!QProcess::startDetached(toqstr(latexEnvCmdPrefix(path)) + cmd_)) {
			state = SystemcallPrivate::Error;
			return;
		}
		QProcess* released = releaseProcess();
		delete released;
	} else if (process_) {
		state = SystemcallPrivate::Starting;
		process_->start(toqstr(latexEnvCmdPrefix(path)) + cmd_);
	}
}


void SystemcallPrivate::processEvents()
{
	if (process_events_) {
		QCoreApplication::processEvents(/*QEventLoop::ExcludeUserInputEvents*/);
	}
}


void SystemcallPrivate::waitAndProcessEvents()
{
	Sleep::millisec(100);
	processEvents();
}


namespace {

bool queryStopCommand(QString const & cmd)
{
	docstring text = bformat(_(
		"The command\n%1$s\nhas not yet completed.\n\n"
		"Do you want to stop it?"), qstring_to_ucs4(cmd));
	return ProgressInterface::instance()->prompt(_("Stop command?"), text,
			1, 1, _("&Stop it"), _("Let it &run")) == 0;
}

}


bool SystemcallPrivate::waitWhile(State waitwhile, bool process_events, int timeout)
{
	if (!process_)
		return false;

	bool timedout = false;
	process_events_ = process_events;

	// Block GUI while waiting,
	// relay on QProcess' wait functions
	if (!process_events_) {
		if (waitwhile == Starting)
			return process_->waitForStarted(timeout);
		if (waitwhile == Running) {
			int bump = 2;
			while (!timedout) {
				if (process_->waitForFinished(timeout))
					return true;
				bool stop = queryStopCommand(cmd_);
				// The command may have finished in the meantime
				if (process_->state() == QProcess::NotRunning)
					return true;
				if (stop) {
					timedout = true;
					process_->kill();
				} else {
					timeout *= bump;
					bump = 3;
				}
			}
		}
		return false;
	}

	// process events while waiting, no timeout
	if (timeout == -1) {
		while (state == waitwhile && state != Error) {
			waitAndProcessEvents();
		}
		return state != Error;
	} 

	// process events while waiting with timeout
	QTime timer;
	timer.start();
	while (state == waitwhile && state != Error && !timedout) {
		waitAndProcessEvents();
		if (timer.elapsed() > timeout) {
			bool stop = queryStopCommand(cmd_);
			// The command may have finished in the meantime
			if (process_->state() == QProcess::NotRunning)
				break;
			if (stop) {
				timedout = true;
				process_->kill();
			} else
				timeout *= 3;
		}
	}
	return (state != Error) && !timedout;
}


SystemcallPrivate::~SystemcallPrivate()
{
	if (out_index_) {
		out_data_[out_index_] = '\0';
		out_index_ = 0;
		cout << out_data_;
	}
	cout.flush();
	if (err_index_) {
		err_data_[err_index_] = '\0';
		err_index_ = 0;
		cerr << err_data_;
	}
	cerr.flush();

	killProcess();
}


void SystemcallPrivate::stdOut()
{
	if (process_) {
		char c;
		process_->setReadChannel(QProcess::StandardOutput);
		while (process_->getChar(&c)) {
			out_data_[out_index_++] = c;
			if (c == '\n' || out_index_ + 1 == buffer_size_) {
				out_data_[out_index_] = '\0';
				out_index_ = 0;
				ProgressInterface::instance()->appendMessage(QString::fromLocal8Bit(out_data_));
				cout << out_data_;
			}
		}
	}
}


void SystemcallPrivate::stdErr()
{
	if (process_) {
		char c;
		process_->setReadChannel(QProcess::StandardError);
		while (process_->getChar(&c)) {
			err_data_[err_index_++] = c;
			if (c == '\n' || err_index_ + 1 == buffer_size_) {
				err_data_[err_index_] = '\0';
				err_index_ = 0;
				ProgressInterface::instance()->appendError(QString::fromLocal8Bit(err_data_));
				cerr << err_data_;
			}
		}
	}
}


void SystemcallPrivate::processStarted()
{
	if (state != Running) {
		state = Running;
		ProgressInterface::instance()->processStarted(cmd_);
	}
}


void SystemcallPrivate::processFinished(int, QProcess::ExitStatus)
{
	if (state != Finished) {
		state = Finished;
		ProgressInterface::instance()->processFinished(cmd_);
	}
}


void SystemcallPrivate::processError(QProcess::ProcessError)
{
	state = Error;
	ProgressInterface::instance()->appendError(errorMessage());
}


QString SystemcallPrivate::errorMessage() const 
{
	if (!process_)
		return "No QProcess available";

	QString message;
	switch (process_->error()) {
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
			message = "An unknown error occurred.";
			break;
	}
	return message;
}


QString SystemcallPrivate::exitStatusMessage() const
{
	if (!process_)
		return "No QProcess available";

	QString message;
	switch (process_->exitStatus()) {
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
	// From Qt's documentation, in regards to QProcess::exitCode(),
	// "This value is not valid unless exitStatus() returns NormalExit"
	if (!process_ || process_->exitStatus() != QProcess::NormalExit)
		return -1;

	return process_->exitCode();
}


QProcess* SystemcallPrivate::releaseProcess()
{
	QProcess* released = process_;
	process_ = 0;
	return released;
}


void SystemcallPrivate::killProcess()
{
	killProcess(process_);
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
