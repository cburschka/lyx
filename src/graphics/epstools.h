// -*- C++ -*-
/**
 * \file epstools.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_EPSTOOLS_H
#define LYX_EPSTOOLS_H

#include <string>

namespace lyx {
namespace support {

class FileName;

}

namespace graphics {

/// read the BoundingBox entry from a ps/eps-file
std::string const readBB_from_PSFile(support::FileName const & file);

} // namespace graphics
} // namespace lyx

#endif
