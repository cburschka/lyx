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

#include "frontends/LyXView.h"


ControlPreamble::ControlPreamble(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d)
{}


void ControlPreamble::apply()
{
	if (!bufferIsAvailable())
		return;

	view().apply();

	buffer()->params.preamble = params();
	buffer()->markDirty();
	lv_.message(_("LaTeX preamble set"));
}


string const & ControlPreamble::params() const
{
	return params_;
}


void ControlPreamble::params(string const & newparams)
{
	params_ = newparams;
}


void ControlPreamble::setParams()
{
	params_ = buffer()->params.preamble;
}


void ControlPreamble::clearParams()
{
	params_.erase();
}
