/**
 * \file ControlButtons.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlButtons.h"
#include "ButtonControllerBase.h"
#include "ViewBase.h"
#include "lyxrc.h"
#include "support/LAssert.h"


ControlButtons::ControlButtons()
	: emergency_exit_(false), is_closing_(false), bc_ptr_(0), view_ptr_(0)
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


bool ControlButtons::IconifyWithMain() const
{
	return lyxrc.dialogs_iconify_with_main;
}


ButtonControllerBase & ControlButtons::bc()
{
	lyx::Assert(bc_ptr_);
	return *bc_ptr_;
}


ViewBase & ControlButtons::view()
{
	lyx::Assert(view_ptr_);
	return *view_ptr_;
}


void ControlButtons::setView(ViewBase & v)
{
	view_ptr_ = &v;
}


void ControlButtons::setButtonController(ButtonControllerBase & bc)
{
	bc_ptr_ = &bc;
}
