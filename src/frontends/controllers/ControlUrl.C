/**
 * \file ControlUrl.C
 * See the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlUrl.h"


ControlUrl::ControlUrl(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d, LFUN_INSERT_URL)
{}
