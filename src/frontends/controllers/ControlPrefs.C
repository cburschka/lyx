/**
 * \file ControlPrefs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlPrefs.h"

//#include "frontends/Dialogs.h"
#include "commandtags.h"
#include "frontends/LyXView.h"

ControlPrefs::ControlPrefs(LyXView & lv, Dialogs & d)
	: ControlDialogBI(lv, d)
{}


void ControlPrefs::apply()
{
	lyxrc = rc_; 
}


void ControlPrefs::save()
{
	lyxrc = rc_;
	lv_.dispatch(FuncRequest(LFUN_SAVEPREFERENCES));
}
