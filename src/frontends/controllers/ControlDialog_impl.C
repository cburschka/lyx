/**
 * \file ControlDialog_impl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlDialog_impl.h"
#include "ControlDialog.tmpl"


ControlDialogBD::ControlDialogBD(LyXView & lv, Dialogs & d)
	: ControlDialog<ControlConnectBD>(lv, d)
{}


ControlDialogBI::ControlDialogBI(LyXView & lv, Dialogs & d)
	: ControlDialog<ControlConnectBI>(lv, d)
{}
