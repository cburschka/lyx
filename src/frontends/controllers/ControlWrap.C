/**
 * \file ControlWrap.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlWrap.h"
#include "funcrequest.h"
#include "lyxlength.h"
#include "insets/insetwrap.h"


ControlWrap::ControlWrap(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlWrap::initialiseParams(string const & data)
{
	InsetWrapParams params;
	InsetWrapMailer::string2params(data, params);
	params_.reset(new InsetWrapParams(params));
	return true;
}


void ControlWrap::clearParams()
{
	params_.reset();
}


void ControlWrap::dispatchParams()
{
	string const lfun = InsetWrapMailer::params2string(params());
	kernel().dispatch(FuncRequest(LFUN_INSET_APPLY, lfun));
}
