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

#include "ControlTabularCreate.h"
#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "lyxfunc.h"
#include "funcrequest.h"


#include "support/lstrings.h"


ControlTabularCreate::ControlTabularCreate(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d)
{}


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
	if (!bufferIsAvailable())
		return;

	view().apply();

	string const val = tostr(params().first) + " " + tostr(params().second);
	lyxfunc().dispatch(FuncRequest(LFUN_TABULAR_INSERT, val));
}
