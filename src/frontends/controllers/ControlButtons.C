/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlButtons.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlButtons.h"
#include "ButtonControllerBase.h"
#include "ViewBase.h"

ControlButtons::ControlButtons()
	: is_closing_(false)
{}


void ControlButtons::ApplyButton()
{
	apply();
	bc().apply();
}


void ControlButtons::OKButton()
{
	is_closing_ = true;
	apply();
	is_closing_ = false;
	hide();
	bc().ok();
}


void ControlButtons::CancelButton()
{
	hide();
	bc().cancel();
}


void ControlButtons::RestoreButton()
{
	update();
	bc().restore();
}
