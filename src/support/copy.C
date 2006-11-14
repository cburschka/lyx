/**
 * \file copy.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include <fstream>

#include "support/lyxlib.h"

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif


namespace lyx {


using std::ifstream;
using std::ofstream;
using std::ios;
using std::string;


bool lyx::support::chmod(string const & file, unsigned long int mode)
{
#ifdef HAVE_CHMOD
	if (::chmod(file.c_str(), mode_t(mode)) != 0)
		return false;
#else
# ifdef WITH_WARNINGS
#  warning "File permissions are ignored on this system."
# endif
#endif
	return true;
}


bool lyx::support::copy(string const & from, string const & to, unsigned long int mode)
{
	ifstream ifs(from.c_str(), ios::binary | ios::in);
	if (!ifs)
		return false;

	if (mode != (unsigned long int)-1) {
		ofstream ofs(to.c_str(), ios::binary | ios::out | ios::trunc);
		if (!ofs)
			return false;
		ofs.close();
		if (!support::chmod(to, mode))
			return false;
	}

	ofstream ofs(to.c_str(), ios::binary | ios::out | ios::trunc);
	if (!ofs)
		return false;

	ofs << ifs.rdbuf();
	return ofs.good();
}


} // namespace lyx
