/**
 * \file qt2/Dialogs.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 */

#include <config.h>

#include "Dialogs.h"
 
LyXView * dialogs_lyxview;

Dialogs::Dialogs(LyXView * lv)
{
	dialogs_lyxview = lv;
 
	// reduce the number of connections needed in
	// dialogs by a simple connection here.
	hideAll.connect(hideBufferDependent);
}


bool Dialogs::tooltipsEnabled(void)
{
	return false;
}
