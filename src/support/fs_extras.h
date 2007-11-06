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

void copy_file(path const & source, path const & target, bool noclobber);

}
}
