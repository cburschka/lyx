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
 * \file ControlBase.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "ButtonController.h"
#include "ControlBase.h"

void ControlBase::ApplyButton()
{
	apply();
	bc().apply();
}


void ControlBase::OKButton()
{
	apply();
	hide();
	bc().ok();
}


void ControlBase::CancelButton()
{
	hide();
	bc().cancel();
}


void ControlBase::RestoreButton()
{
	update();
	bc().undoAll();
}
