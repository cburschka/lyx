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
	static string const & binpath() {return binpath_;}
	//
	static string const & binname() {return binname_;}
	//
	static string const & homepath() {return homepath_;}
	//
	static string const & nulldev() {return nulldev_;}

	// system_tempdir actually doesn't belong here.
	// I put it here only to avoid a global variable.
	static void setTmpDir(string p) {tmpdir_ = p;}

	//
	static string const & getTmpDir() {return tmpdir_;}

	//
	static string current_root();

	//
	static os::shell_type shell() {return _shell;}

	// DBCS aware!
	static string::size_type common_path(string const &p1,
					     string const &p2);

	// Converts a unix style path to host OS style.
	static string external_path(string const &p);
	// Converts a host OS style path to unix style.
	static string internal_path(string const &p);
	// is path absolute?
	static bool is_absolute_path(string const & p);
	// returns a string suitable to be passed to fopen when
	// reading a file
	static char const * read_mode();
	// same for popen().
	static char const * popen_read_mode();
	//
	static void warn(string mesg);
private:
	static string binpath_;
	static string binname_;
	static string tmpdir_;
	static string homepath_;
	static string nulldev_;
	static os::shell_type _shell;
	// Used only on OS/2 to determine file system encoding.
	static unsigned long cp_;

	// Never initialize static variables in the header!
	// Anyway I bet this class will never be constructed.
	os() {};

	// Ignore warning!
	~os() {};

};

#endif
