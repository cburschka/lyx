/**
 * \file rename.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lyxlib.h"
#include "support/FileName.h"

#include <cstdio>


namespace lyx {
namespace support {


using std::string;


bool rename(FileName const & from, FileName const & to)
{
	if (::rename(from.toFilesystemEncoding().c_str(), to.toFilesystemEncoding().c_str()) == -1) {
		if (copy(from, to)) {
			from.removeFile();
			return true;
		}
		return false;
	}
	return true;
}


} // namespace support
} // namespace lyx
