/**
 * \file ControlIndex.C
 * Read the file COPYING
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlIndex.h"


ControlIndex::ControlIndex(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d, LFUN_INDEX_INSERT)
{}
