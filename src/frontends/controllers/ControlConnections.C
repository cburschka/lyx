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
 * \file ControlConnections.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "ControlConnections.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "buffer.h"

using SigC::slot;

ControlConnectBase::ControlConnectBase(LyXView & lv, Dialogs & d)
	: lv_(lv), d_(d), h_(0), r_(0)
{}


void ControlConnectBase::connect()
{
	r_ = Dialogs::redrawGUI.
		connect(slot(this, &ControlConnectBase::redraw));
}

void ControlConnectBase::disconnect()
{
	h_.disconnect();
	r_.disconnect();
}


void ControlConnectBase::redraw()
{
	view().redraw();
}


bool ControlConnectBase::isReadonly() const
{
	if (!lv_.buffer())
		return true;

	return lv_.buffer()->isReadonly();
}


ControlConnectBI::ControlConnectBI(LyXView & lv, Dialogs & d)
        : ControlConnectBase(lv, d)
{}


void ControlConnectBI::connect()
{
	h_ = d_.hideAll.connect(slot(this, &ControlConnectBI::hide));
	ControlConnectBase::connect();
}

ControlConnectBD::ControlConnectBD(LyXView & lv, Dialogs & d)
	: ControlConnectBase(lv, d),
	  u_(0)
{}


void ControlConnectBD::connect()
{
	u_ = d_.updateBufferDependent.
		connect(slot(this, &ControlConnectBD::updateSlot));
	h_ = d_.hideBufferDependent.
		connect(slot(this, &ControlConnectBD::hide));
	ControlConnectBase::connect();
}

void ControlConnectBD::disconnect()
{
	u_.disconnect();
	ControlConnectBase::disconnect();
}

