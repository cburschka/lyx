/**
 * \file LAssert.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LAssert.h"
#include "support/lyxlib.h"

#ifdef ENABLE_ASSERTIONS
#include "lyx_main.h"

namespace lyx {
namespace support {

namespace {

void emergencyCleanup()
{
	static bool didCleanup;
	if (didCleanup)
		return;

	didCleanup = true;

	LyX::emergencyCleanup();
}

} // namespace anon

void Assert(bool assertion)
{
	if (!assertion) {
		emergencyCleanup();
		lyx::support::abort();
	}
}

} // namespace support
} // namespace lyx

#endif
