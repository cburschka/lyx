// os.h copyright "Ruurd A. Reitsma" <R.A.Reitsma@wbmt.tudelft.nl>

#ifndef OS_H
#define OS_H

#ifdef __GNUG__
#pragma interface
#endif

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
	static void init(int * argc, char ** argv[]);

	// 
	static string binpath() {return binpath_;};

	// 
	static string binname() {return binname_;};

	// system_tempdir actually doesn't belong here.
	// I put it here only to avoid a global variable.
	static void setTmpDir(string p) {tmpdir_ = p;};

	// 
	static string getTmpDir() {return tmpdir_;};

	// 
	static string current_root();

	// 
	static os::shell_type shell() {return _shell;};

	// DBCS aware!
	static string::size_type common_path(string const &p1,
					     string const &p2);

	// no-op on UNIX, '\\'->'/' on OS/2 and Win32, ':'->'/' on MacOS, etc.
	static string slashify_path(string p);
	// converts a host OS path to unix style
	static string external_path(string const &p);
	// converts a unix path to host OS style
	static string internal_path(string const &p);
	// is path absolute?
	static bool is_absolute_path(string const & p);
	
	//
	static void warn(string mesg);
private:
	static string binpath_;
	static string binname_;
	static string tmpdir_;
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
