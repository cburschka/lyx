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
#include <utility>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlPrint.h"
#include "buffer.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "PrinterParams.h"
#include "Liason.h"
#include "helper_funcs.h"
#include "frontends/Alert.h"
#include "gettext.h"
#include "BufferView.h"
#include "support/LAssert.h"

using Liason::printBuffer;
using Liason::getPrinterParams;
using std::make_pair;

ControlPrint::ControlPrint(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d),
	  params_(0)
{
	d_.showPrint.connect(SigC::slot(this, &ControlPrint::show));
}


void ControlPrint::apply()
{
	if (!lv_.view()->available())
		return;

	view().apply();

	if (!printBuffer(lv_.buffer(), params())) {
		Alert::alert(_("Error:"),
			   _("Unable to print"),
			   _("Check that your parameters are correct"));
	}
}


PrinterParams & ControlPrint::params() const
{
	lyx::Assert(params_);
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


string const ControlPrint::Browse(string const & in_name)
{
	string const title = N_("Print to file");
	string const pattern = "*.ps";

	// Show the file browser dialog
	return browseRelFile(&lv_, in_name, lv_.buffer()->filePath(),
			     title, pattern);
}
