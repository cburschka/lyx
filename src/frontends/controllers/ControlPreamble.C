/**
 * \file ControlPreamble.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
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
