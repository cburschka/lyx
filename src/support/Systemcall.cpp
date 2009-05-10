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
	process.waitForFinished();
	return process.exitCode();
#endif

	return ::system(command.c_str());
}

} // namespace support
} // namespace lyx
