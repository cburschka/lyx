#include <config.h>

#include <fstream>

//#include <stdio.h>

#include "support/lyxlib.h"
#include "LString.h"
//#include "support/syscall.h"
#include "support/filetools.h"

bool lyx::copy(string const & from, string const & to)
{
#if 0
	string command = "cp " + QuoteName(from) + " " + QuoteName(to);
	return Systemcalls().startscript(Systemcalls::System,
					 command) == 0;
#else
	ifstream ifs(from.c_str());
	if (!ifs) return false;
	ofstream ofs(to.c_str(), ios::out|ios::trunc);
	if (!ofs) return false;
	ofs << ifs.rdbuf();
	if (ofs.good()) return true;
	return false;
#endif
}
