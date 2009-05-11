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

#include <QProcess>

using namespace std;

namespace lyx {
namespace support {

// Reuse of instance
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

//#define DISABLE_EVALUATE_QPROCESS
#ifndef DISABLE_EVALUATE_QPROCESS
	QString cmd = QString::fromLocal8Bit(command.c_str());
	QProcess process;
	process.start(cmd);
	if (!process.waitForStarted(1000)) {
		LYXERR0("Qprocess " << cmd << " did not start!");
		LYXERR0("error " << process.error());
		LYXERR0("state " << process.state());
		LYXERR0("status " << process.exitStatus());
		return 10;
	}
	if (!process.waitForFinished(30000)) {
		LYXERR0("Qprocess " << cmd << " did not finished!");
		LYXERR0("error " << process.error());
		LYXERR0("state " << process.state());
		LYXERR0("status " << process.exitStatus());
		return 20;
	}
	if (process.exitCode()) {
		LYXERR0("Qprocess " << cmd << " finished!");
		LYXERR0("exitCode " << process.exitCode());
		LYXERR0("error " << process.error());
		LYXERR0("state " << process.state());
		LYXERR0("status " << process.exitStatus());
	}
	return process.exitCode();
#endif

	return ::system(command.c_str());
}

} // namespace support
} // namespace lyx
