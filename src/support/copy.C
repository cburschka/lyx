#include <config.h>

#include <fstream>

#include "support/lyxlib.h"
#include "LString.h"

bool lyx::copy(string const & from, string const & to)
{
	std::ifstream ifs(from.c_str());
	if (!ifs) return false;
	std::ofstream ofs(to.c_str(),
			  std::ios::binary | std::ios::out | std::ios::trunc);
	if (!ofs) return false;
	ofs << ifs.rdbuf();
	if (ofs.good()) return true;
	return false;
}
