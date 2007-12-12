/**
 * \file copy.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include <fstream>

#include "support/FileName.h"
#include "support/lyxlib.h"

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

using namespace std;

namespace lyx {
namespace support {

bool chmod(FileName const & file, unsigned long int mode)
{
#if defined (HAVE_CHMOD) && defined (HAVE_MODE_T)
	if (::chmod(file.toFilesystemEncoding().c_str(), mode_t(mode)) != 0)
		return false;
#else
	// FIXME: "File permissions are ignored on this system."
#endif
	return true;
}


bool copy(FileName const & from, FileName const & to, unsigned long int mode)
{
	ifstream ifs(from.toFilesystemEncoding().c_str(), ios::binary | ios::in);
	if (!ifs)
		return false;

	if (mode != (unsigned long int)-1) {
		ofstream ofs(to.toFilesystemEncoding().c_str(),
			ios::binary | ios::out | ios::trunc);
		if (!ofs)
			return false;
		ofs.close();
		if (!support::chmod(to, mode))
			return false;
	}

	ofstream ofs(to.toFilesystemEncoding().c_str(),
			ios::binary | ios::out | ios::trunc);
	if (!ofs)
		return false;

	ofs << ifs.rdbuf();
	return ofs.good();
}

} // namespace support
} // namespace lyx
