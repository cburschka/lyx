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
#include "BufferView.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "lyxfunc.h"

ControlTabularCreate::ControlTabularCreate(LyXView & lv, Dialogs & d)
	: ControlDialog<ControlConnectBD>(lv, d)
{
	d_.showTabularCreate.connect(SigC::slot(this,
						&ControlTabularCreate::show));
}


string & ControlTabularCreate::params() const
{
	Assert(params_);
	return *params_;
}


void ControlTabularCreate::setParams()
{
	if (params_) delete params_;
	params_ = new string;

	bc().valid(); // so that the user can press Ok
}


void ControlTabularCreate::clearParams()
{
	if (params_) {
		delete params_;
		params_ = 0;
	}
}


void ControlTabularCreate::apply()
{
	if (!lv_.view()->available())
		return;
   
	view().apply();

	lv_.getLyXFunc()->Dispatch(LFUN_INSET_TABULAR, params());
}
