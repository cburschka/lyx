/**
 * \file getcwd.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lyxlib.h"

namespace lyx {
namespace support {

// Returns current working directory
FileName const getcwd()
{
	return FileName(".");
}

} // namespace support
} // namespace lyx
