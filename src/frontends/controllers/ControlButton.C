// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlButton.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlButton.h"

void ControlButton::ApplyButton()
{
	apply();
	bc().apply();
}


void ControlButton::OKButton()
{
	apply();
	hide();
	bc().ok();
}


void ControlButton::CancelButton()
{
	hide();
	bc().cancel();
}


void ControlButton::RestoreButton()
{
	update();
	bc().undoAll();
}
