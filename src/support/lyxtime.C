/**
 * \file lyxtime.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyxtime.h"

namespace lyx {

time_type current_time()
{
	return time(0);
}

} // namespace lyx
