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

using namespace lyx::support;

ViewBase::ViewBase(string const & t)
	: controller_ptr_(0), title_(t)
{}


void ViewBase::setController(ControlButtons & c)
{
	controller_ptr_ = &c;
}


void ViewBase::setTitle(string const & newtitle)
{
	title_ = newtitle;
}


string const & ViewBase::getTitle() const
{
	return title_;
}


ControlButtons & ViewBase::getController()
{
	Assert(controller_ptr_);
	return *controller_ptr_;
}


ControlButtons const & ViewBase::getController() const
{
	Assert(controller_ptr_);
	return *controller_ptr_;
}


ButtonController & ViewBase::bc()
{
	return getController().bc();
}
