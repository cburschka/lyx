/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlUrl.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlUrl.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "buffer.h"

using SigC::slot;

ControlUrl::ControlUrl(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d, LFUN_INSERT_URL)
{
	d_.showUrl.connect(slot(this, &ControlUrl::showInset));
	d_.createUrl.connect(slot(this, &ControlUrl::createInset));
}
