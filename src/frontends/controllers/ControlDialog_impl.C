// -*- C++ -*-
/*
 * \file ControlDialog_impl.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#include "ControlDialog_impl.h"
#include "ControlDialog.tmpl"

ControlDialogBD::ControlDialogBD(LyXView & lv, Dialogs & d)
	: ControlDialog<ControlConnectBD>(lv, d)
{}


ControlDialogBI::ControlDialogBI(LyXView & lv, Dialogs & d)
	: ControlDialog<ControlConnectBD>(lv, d)
{}
