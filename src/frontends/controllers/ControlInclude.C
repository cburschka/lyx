/**
 * \file ControlInclude.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "ControlInclude.h"
#include "buffer.h"
#include "Dialogs.h"
#include "lyxfunc.h"
#include "LyXView.h"

using SigC::slot;

ControlInclude::ControlInclude(LyXView & lv, Dialogs & d)
	: ControlInset<InsetInclude, InsetInclude::Params>(lv, d)
{
	d_.showInclude.connect(slot(this, &ControlInclude::showInset));
}

LyXView * ControlInclude::lv() const
{
	return &lv_;
}


void ControlInclude::applyParamsToInset()
{
	inset()->set(params());
	lv_.view()->updateInset(inset(), true);
}
