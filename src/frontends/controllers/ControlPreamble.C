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

#include "buffer.h"
#include "gettext.h"

#include "frontends/Liason.h"


ControlPreamble::ControlPreamble(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d),
	  params_(0)
{}


void ControlPreamble::apply()
{
	if (!bufferIsAvailable())
		return;

	view().apply();

	buffer()->params.preamble = params();
	buffer()->markDirty();
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
	params_ = new string(buffer()->params.preamble);
}


void ControlPreamble::clearParams()
{
	delete params_;
	params_ = 0;
}
