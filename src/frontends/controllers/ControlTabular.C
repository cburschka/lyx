// -*- C++ -*-
/**
 * \file ControlTabular.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlTabular.h"
#include "funcrequest.h"
#include "lyxrc.h"
#include "insets/insettabular.h"
#include "support/LAssert.h"


ControlTabular::ControlTabular(Dialog & parent)
	: Dialog::Controller(parent), active_cell_(-1)
{}


void ControlTabular::initialiseParams(string const & data)
{
	Buffer * buffer = kernel().buffer();
	if (!buffer)
		return;

	InsetTabular tmp(*buffer);
	int cell = InsetTabularMailer::string2params(data, tmp);
	if (cell != -1) {
		params_.reset(new LyXTabular(*tmp.tabular.get()));
		active_cell_ = cell;
	}
}


void ControlTabular::clearParams()
{
	params_.reset();
	active_cell_ = -1;
}


int ControlTabular::getActiveCell() const
{
	return active_cell_;
}


LyXTabular const & ControlTabular::tabular() const
{
	lyx::Assert(params_.get());
	return *params_.get();
}


void ControlTabular::set(LyXTabular::Feature f, string const & arg)
{
	string const data = featureAsString(f) + ' ' + arg;
	kernel().dispatch(FuncRequest(LFUN_TABULAR_FEATURE, data));
}


bool ControlTabular::useMetricUnits() const
{
	return lyxrc.default_papersize > BufferParams::PAPER_EXECUTIVEPAPER;
}
