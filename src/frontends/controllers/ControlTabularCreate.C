/**
 * \file ControlTabularCreate.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlTabularCreate.h"
#include "funcrequest.h"
#include "support/lstrings.h"


ControlTabularCreate::ControlTabularCreate(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlTabularCreate::initialiseParams(string const &)
{
	params_.first = params_.second = 5;
	return true;
}


void ControlTabularCreate::clearParams()
{
	params_.first = params_.second = 0;
}


void ControlTabularCreate::dispatchParams()
{
	string data = tostr(params().first) + ' ' + tostr(params().second);
	kernel().dispatch(FuncRequest(LFUN_TABULAR_INSERT, data));
}
