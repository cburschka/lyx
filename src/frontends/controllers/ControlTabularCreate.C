/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlTabularCreate.C
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlTabularCreate.h"
#include "BufferView.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "support/lstrings.h"

ControlTabularCreate::ControlTabularCreate(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d)
{
	d_.showTabularCreate.connect(SigC::slot(this,
						&ControlTabularCreate::show));
}


ControlTabularCreate::rowsCols & ControlTabularCreate::params()
{
		return params_;
}


void ControlTabularCreate::setParams()
{
	bc().valid(); // so that the user can press Ok
}


void ControlTabularCreate::apply()
{
	if (!lv_.view()->available())
		return;
   
	view().apply();

	string const val(tostr(params().first) + " " + tostr(params().second)); 
 
	lv_.getLyXFunc()->dispatch(LFUN_INSET_TABULAR, val);
}
