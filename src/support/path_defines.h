// -*- C++ -*-
/**
 * \file path_defines.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef PATH_DEFINES_H
#define PATH_DEFINES_H

#include "LString.h"

namespace lyx {
namespace support {

/** \returns true if the user lyx dir existed already and did not need
 *  to be created afresh.
 */
bool setLyxPaths();

} // namespace support
} // namespace lyx

#endif // NOT PATH_DEFINES_H
