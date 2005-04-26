// -*- C++ -*-
/* \file fs_extras.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <boost/filesystem/path.hpp>

namespace boost {
namespace filesystem {

bool is_readable(path const & ph);

bool is_writable(path const & ph);

bool is_readonly(path const & ph);

void copy_file(path const & source, path const & target, bool noclobber);

}
}
