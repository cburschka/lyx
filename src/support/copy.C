#include <config.h>

#include <fstream>

#include "support/lyxlib.h"
#include "LString.h"

using std::ifstream;
using std::ofstream;
using std::ios;

bool lyx::copy(string const & from, string const & to)
{
	ifstream ifs(from.c_str(), ios::binary | ios::in);
	if (!ifs)
		return false;

	ofstream ofs(to.c_str(), ios::binary | ios::out | ios::trunc);
	if (!ofs)
		return false;

	ofs << ifs.rdbuf();
	return ofs.good();
}
