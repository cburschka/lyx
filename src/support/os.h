// os.h copyright "Ruurd A. Reitsma" <rareitsma@yahoo.com>

#ifndef OS_H
#define OS_H

#include "LString.h"

/// Do we need a base class for this?
class os {
public:
	//
	enum shell_type {
		UNIX,	// Do we have to distinguish sh and csh?
		CMD_EXE
	};

	//
	static void init(int argc, char * argv[]);

	//
	static string const & nulldev() {return nulldev_;}

	//
	static string current_root();

	//
	static os::shell_type shell() {return shell_;}

	// DBCS aware!
	static string::size_type common_path(string const &p1,
					     string const &p2);

	/// Converts a unix style path to host OS style.
	static string external_path(string const &p);
	/// Converts a host OS style path to unix style.
	static string internal_path(string const &p);
	/// Is the path absolute?
	static bool is_absolute_path(string const & p);
	/** Returns a string suitable to be passed to fopen when
	 *  reading a file.
	 */
	static char const * read_mode();
	/** Returns a string suitable to be passed to popen when
	 *  reading a file.
	 */
	static char const * popen_read_mode();

	/** The character used to separate paths returned by the
	 *  PATH environment variable.
	 */
	static char path_separator();
  
	/** If @c use_cygwin_paths is true, LyX will output cygwin style paths
	 *  rather than native Win32 ones. Obviously, this option is used only
	 *  under Cygwin.
	 */
	static void cygwin_path_fix(bool use_cygwin_paths);

private:
	static string const nulldev_;
	static os::shell_type shell_;
#ifdef __EMX__
	// Used only on OS/2 to determine file system encoding.
	static unsigned long cp_;
#endif

	// Never initialize static variables in the header!
	// Anyway I bet this class will never be constructed.
	os() {};

	// Ignore warning!
	~os() {};

};

#endif
