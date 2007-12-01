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

bool rename(FileName const & from, FileName const & to)
{
	if (::rename(from.toFilesystemEncoding().c_str(),
			to.toFilesystemEncoding().c_str()) != -1) 
		return true;
	if (!copy(from, to))
		return false;
	from.removeFile();
	return true;
}


} // namespace support
} // namespace lyx
