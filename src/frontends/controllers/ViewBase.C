/**
 * \file ViewBase.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ViewBase.h"
#include "ControlButtons.h"
#include "support/LAssert.h"


ViewBase::ViewBase()
	: controller_ptr_(0)
{}


void ViewBase::setController(ControlButtons & c)
{
	controller_ptr_ = &c;
}


ControlButtons & ViewBase::getController()
{
	lyx::Assert(controller_ptr_);
	return *controller_ptr_;
}


ControlButtons const & ViewBase::getController() const
{
	lyx::Assert(controller_ptr_);
	return *controller_ptr_;
}


ButtonController & ViewBase::bc()
{
	return getController().bc();
}
