/**
 * \file abort.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include <stdlib.h>

#include "support/lyxlib.h"

void lyx::abort()
{
	::abort();
}
