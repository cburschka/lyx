/**
 * \file ControlPrint.C
 * See the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlPrint.h"

#include "ViewBase.h"
#include "ButtonControllerBase.h"

#include "buffer.h"
#include "gettext.h"
#include "helper_funcs.h"
#include "PrinterParams.h"

#include "frontends/Alert.h"
#include "frontends/Liason.h"

#include "support/LAssert.h"

using Liason::printBuffer;
using Liason::getPrinterParams;


ControlPrint::ControlPrint(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d),
	  params_(0)
{}


void ControlPrint::apply()
{
	if (!bufferIsAvailable())
		return;

	view().apply();

	if (!printBuffer(buffer(), params())) {
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
	params_ = new PrinterParams(getPrinterParams(buffer()));

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
	string const title = _("Print to file");
	string const pattern = "*.ps";

	// Show the file browser dialog
	return browseRelFile(&lv_, in_name, buffer()->filePath(),
			     title, pattern);
}
