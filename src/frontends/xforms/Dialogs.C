/**
 * \file xforms/Dialogs.C
 * Copyright 1995 Matthias Ettrich
 * Copyright 1995-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Allan Rae, rae@lyx.org
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"
#include "Tooltips.h"

LyXView * dialogs_lyxview;


/// Are the tooltips on or off?
bool Dialogs::tooltipsEnabled()
{
	return Tooltips::enabled();
}


Dialogs::Dialogs(LyXView * lv)
{
#if 1
	dialogs_lyxview = lv;
#endif
	// reduce the number of connections needed in
	// dialogs by a simple connection here.
	hideAll.connect(hideBufferDependent);
}
