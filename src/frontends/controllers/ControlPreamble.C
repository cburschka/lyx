/**
 * \file FormPreamble.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlPreamble.h"
#include "ViewBase.h"
#include "frontends/LyXView.h"
#include "buffer.h"
#include "Liason.h"
#include "gettext.h"
#include "BufferView.h"
#include "support/LAssert.h"


ControlPreamble::ControlPreamble(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d),
	  params_(0)
{}


void ControlPreamble::apply()
{
	if (!lv_.view()->available())
		return;

	view().apply();

	lv_.buffer()->params.preamble = params();
	lv_.buffer()->markDirty();
	Liason::setMinibuffer(&lv_, _("LaTeX preamble set"));
}


string & ControlPreamble::params() const
{
	lyx::Assert(params_);
	return *params_;
}


void ControlPreamble::setParams()
{
	delete params_;
	params_ = new string(lv_.buffer()->params.preamble);
}


void ControlPreamble::clearParams()
{
	delete params_;
	params_ = 0;
}
