// os_unix.C

// Various OS specific functions
#include <config.h>

#include "os.h"
#include "support/filetools.h"

string os::binpath_ = string();
string os::binname_ = string();
string os::tmpdir_ = string();
os::shell_type os::_shell = os::UNIX;
unsigned long os::cp_ = 0;

void os::init(int * /*argc*/, char ** argv[]) /* :cp_(0), _shell(os::UNIX) */ {
	static bool initialized = false;

	if (initialized)
		return;

	initialized = true;
	string tmp = *argv[0];
	binname_ = OnlyFilename(tmp);
	tmp = ExpandPath(tmp); // This expands ./ and ~/
	if (!AbsolutePath(tmp)) {
		string binsearchpath = GetEnvPath("PATH");
		// This will make "src/lyx" work always :-)
		binsearchpath += ";.";
		tmp = FileOpenSearch(binsearchpath, tmp);
	}

	tmp = MakeAbsPath(OnlyPath(tmp));

	// In case we are running in place and compiled with shared libraries
	if (suffixIs(tmp, "/.libs/"))
		tmp.erase(tmp.length()-6, string::npos);
	binpath_ = tmp;
}

void os::warn(string /*mesg*/) {
	return;
}

string os::current_root() {
	return string("/");
}

string::size_type os::common_path(string const &p1, string const &p2) {
	string::size_type i = 0,
			p1_len = p1.length(),
			p2_len = p2.length();
	while (i < p1_len && i < p2_len && p1[i] == p2[i]) ++i;
	if ((i < p1_len && i < p2_len)
	    || (i < p1_len && p1[i] != '/' && i == p2_len)
	    || (i < p2_len && p2[i] != '/' && i == p1_len)) {
		if (i) --i;     // here was the last match
		while (i && p1[i] != '/') --i;
	}
	return i;
}

string os::slashify_path(string p) {
	return p;
}

string os::external_path(string p) {
	return p;
}

