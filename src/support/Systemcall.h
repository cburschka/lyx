// -*- C++ -*-
/**
 * \file Systemcall.h
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

#ifndef SYSTEMCALL_H
#define SYSTEMCALL_H

#include <string>

namespace lyx {
namespace support {

/**
 * An instance of Class Systemcall represents a single child process.
 *
 * Class Systemcall uses system() to launch the child process.
 * The user can choose to wait or not wait for the process to complete, but no
 * callback is invoked upon completion of the child.
 *
 * The child process is not killed when the Systemcall instance goes out of
 * scope.
 */
class Systemcall {
public:
	/// whether to wait for completion
	enum Starttype {
		Wait, //< wait for completion before returning from startscript()
		DontWait //< don't wait for completion
	};

	/** Start child process.
	 *  The string "what" contains a commandline with arguments separated
	 *  by spaces and encoded in the filesystem encoding. "$$s" will be
	 *  replaced accordingly by libScriptSearch(). The string "path"
	 *  contains the path to be prepended to the TEXINPUTS environment
	 *  variable and encoded in the path to be prepended to the TEXINPUTS
	 *  environment variable and utf-8. Unset "process_events" in case UI
	 *  should be blocked while processing the external command.
	 */
	int startscript(Starttype how, std::string const & what,
			std::string const & path = empty_string(),
			bool process_events = false);
};

} // namespace support
} // namespace lyx

#endif // SYSTEMCALL_H
