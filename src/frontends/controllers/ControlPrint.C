/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 2001 The LyX Team.
 *
 *======================================================
 *
 * \file ControlPrint.C
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlPrint.h"
#include "buffer.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "lyxrc.h"
#include "PrinterParams.h"
#include "Liason.h"

#include "lyx_gui_misc.h" // WriteAlert

using Liason::printBuffer;
using Liason::getPrinterParams;

ControlPrint::ControlPrint(LyXView & lv, Dialogs & d)
	: ControlDialog<ControlConnectBD>(lv, d)
{
	d_.showPrint.connect(SigC::slot(this, &ControlPrint::show));
}


void ControlPrint::apply()
{
	if (!lv_.view()->available())
		return;
   
	view().apply();

	if (!printBuffer(lv_.buffer(), params())) {
		WriteAlert(_("Error:"),
			   _("Unable to print"),
			   _("Check that your parameters are correct"));
	}
}


LyXView * ControlPrint::lv() const
{
	return &lv_;
}


PrinterParams & ControlPrint::params() const
{
	Assert(params_);
	return *params_;
}


void ControlPrint::setParams()
{
	if (params_) delete params_;
	params_ = new PrinterParams(getPrinterParams(lv_.buffer()));

	bc().valid(); // so that the user can press Ok
}


void ControlPrint::clearParams()
{
	if (params_) {
		delete params_;
		params_ = 0;
	}
}


