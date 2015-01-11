/**
 * \file ConsoleApplication.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONSOPLEAPPLICATION_H
#define CONSOPLEAPPLICATION_H

#include "support/strfwd.h"

namespace lyx {

namespace support {

class ConsoleApplicationPrivate;

/// The main application class for console mode.
/**
There should be only one instance of this class. No Qt object
initialisation should be done before the instantiation of this class.
This class could be moved to src/frontends/console in the future.
This would make sense if more console app related code is created. 
*/
class ConsoleApplication
{
	friend class ConsoleApplicationPrivate;
public:
	ConsoleApplication(std::string const & app, int & argc, char ** argv);
	virtual ~ConsoleApplication();

	/// Start the event loop and execute the application
	int exec();

protected:
	/// Do the real work.
	/// This is called after the event loop was started.
	virtual void doExec() = 0;
	/**
	 * Exit the application with status \p status.
	 * This must be called from doExec(), otherwise the application runs
	 * forever.
	 * Note that in contrast to the ISO C function ::exit() this method does
	 * return. It only registers that the program is to be stopped with the
	 * given status code, and this happens the next time the event loop is
	 * processed.
	 */
	void exit(int status);

private:
	ConsoleApplicationPrivate * const d;
};

} // namespace support
} // namespace lyx

#endif // CONSOPLEAPPLICATION_H
