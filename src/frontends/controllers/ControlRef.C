/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlRef.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlRef.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "buffer.h"
#include "lyxfunc.h"

using SigC::slot;

ControlRef::ControlRef(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d, LFUN_REF_INSERT)
{
	d_.showRef.connect(slot(this, &ControlRef::showInset));
	d_.createRef.connect(slot(this, &ControlRef::createInset));
}


std::vector<string> const ControlRef::getLabelList() const
{
	return lv_.buffer()->getLabelList();
}


void ControlRef::gotoRef(string const & ref) const
{
	lv_.getLyXFunc()->dispatch(LFUN_BOOKMARK_SAVE, "0");
	lv_.getLyXFunc()->dispatch(LFUN_REF_GOTO, ref);
}


void ControlRef::gotoBookmark() const
{
	lv_.getLyXFunc()->dispatch(LFUN_BOOKMARK_GOTO, "0");
}

