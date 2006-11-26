/**
 * \file unlink.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lyxlib.h"
#include "support/filename.h"


namespace lyx {
namespace support {

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

int unlink(FileName const & pathname)
{
	return ::unlink(pathname.toFilesystemEncoding().c_str());
}


} // namespace support
} // namespace lyx
