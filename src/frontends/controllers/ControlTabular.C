// -*- C++ -*-
/**
 * \file ControlTabular.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include "ControlTabular.h"

#include "ButtonControllerBase.h"
#include "ViewBase.h"
#include "support/LAssert.h"

#include "frontends/LyXView.h"
#include "BufferView.h"
#include "buffer.h"
#include "lyxrc.h"
#include "insets/insettabular.h" 

#include <boost/bind.hpp>

ControlTabular::ControlTabular(LyXView & lv, Dialogs & d)
	: ControlConnectBD(lv, d),
	  inset_(0), dialog_built_(false)
{}


void ControlTabular::showInset(InsetTabular * inset)
{
	lyx::Assert(inset);

	connectInset(inset);
	show(inset);

	// The widgets may not be valid, so refresh the button controller
	// FIXME: needed ?
	bc().refresh();
}


void ControlTabular::updateInset(InsetTabular * inset)
{
	lyx::Assert(inset);

	connectInset(inset);

	if (!dialog_built_) {
		view().build();
		dialog_built_ = true;
	}
 
	update();
}

 
void ControlTabular::show(InsetTabular * inset)
{
	inset_ = inset;

	if (emergency_exit_) {
		hide();
		return;
	}

	if (!dialog_built_) {
		view().build();
		dialog_built_ = true;
	}

	bc().readOnly(bufferIsReadonly());
	view().show();
}


void ControlTabular::update()
{
	if (emergency_exit_) {
		hide();
		return;
	}

	bc().readOnly(bufferIsReadonly());
	view().update();

	// The widgets may not be valid, so refresh the button controller
	// FIXME: needed ? 
	bc().refresh();
}


void ControlTabular::hide()
{
	emergency_exit_ = false;
	inset_ = 0;

	ih_.disconnect();
	disconnect();
	view().hide();
}


void ControlTabular::updateSlot(bool switched)
{
	if (switched)
		hide();
	else
		update();
}


void ControlTabular::connectInset(InsetTabular * inset)
{
	// If connected to another inset, disconnect from it.
	if (inset_) {
		ih_.disconnect();
		inset_ = 0;
	}

	if (inset) {
		inset_ = inset;
		ih_ = inset->hideDialog.connect(
			boost::bind(&ControlTabular::hide, this));
	}
	connect();
}
 
 
InsetTabular * ControlTabular::inset() const
{
	lyx::Assert(inset_);
	return inset_;
}


LyXTabular * ControlTabular::tabular() const
{
	lyx::Assert(inset_);
	return inset_->tabular.get();
}

 
void ControlTabular::set(LyXTabular::Feature f, string const & arg)
{
	lyx::Assert(inset_);
	inset_->tabularFeatures(lv_.view().get(), f, arg);
}

 
bool ControlTabular::metric() const
{
	return lyxrc.default_papersize > BufferParams::PAPER_EXECUTIVEPAPER;
}
