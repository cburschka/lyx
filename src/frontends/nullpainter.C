/**
 * \file nullpainter.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "nullpainter.h"

#include "LColor.h"

#include <limits>


namespace lyx {
namespace frontend {

int NullPainter::paperHeight() const
{
	return std::numeric_limits<int>::max();
}

} // namespace frontend
} // namespace lyx
