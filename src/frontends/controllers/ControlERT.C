/**
 * \file ControlERT.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlERT.h"
#include "funcrequest.h"


ControlERT::ControlERT(Dialog & parent)
	: Dialog::Controller(parent), status_(InsetERT::Collapsed)
{}


void ControlERT::initialiseParams(string const & data)
{
	InsetERTMailer::string2params(data, status_);
}


void ControlERT::clearParams()
{
	status_ = InsetERT::Collapsed;
}


void ControlERT::dispatchParams()
{
	FuncRequest fr(LFUN_ERT_APPLY, InsetERTMailer::params2string(status_));
	kernel().dispatch(fr);
}
