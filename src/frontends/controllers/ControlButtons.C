/**
 * \file ControlButtons.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>


#include "ControlButtons.h"
#include "ButtonController.h"
#include "BCView.h"
#include "lyxrc.h"


ControlButtons::ControlButtons()
	: emergency_exit_(false), is_closing_(false),
	  bc_ptr_(new ButtonController), view_ptr_(0)
{}


ControlButtons::~ControlButtons()
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


ButtonController & ControlButtons::bc()
{
	BOOST_ASSERT(bc_ptr_.get());
	return *bc_ptr_.get();
}


ViewBase & ControlButtons::view()
{
	BOOST_ASSERT(view_ptr_);
	return *view_ptr_;
}


void ControlButtons::setView(ViewBase & v)
{
	view_ptr_ = &v;
}
