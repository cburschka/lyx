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

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlConnections.h"
#include "ViewBase.h"
#include "Dialogs.h"
#include "buffer.h"

#include "frontends/LyXView.h"

#include <boost/bind.hpp>

ControlConnectBase::ControlConnectBase(LyXView & lv, Dialogs & d)
	: lv_(lv), d_(d)
{}


void ControlConnectBase::connect()
{
	r_ = d_.redrawGUI.
		connect(boost::bind(&ControlConnectBase::redraw, this));
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


ControlConnectBase::DocTypes ControlConnectBase::docType() const
{
	if (!lv_.buffer())
		return LATEX;

	if (lv_.buffer()->isLatex())
		return LATEX;
	else if (lv_.buffer()->isLiterate())
		return LITERATE;
	else if (lv_.buffer()->isLinuxDoc())
		return LINUXDOC;
	/* else if (lv_.buffer()->isDocBook()) */
		return DOCBOOK;
}


ControlConnectBI::ControlConnectBI(LyXView & lv, Dialogs & d)
	: ControlConnectBase(lv, d)
{}


void ControlConnectBI::connect()
{
	h_ = d_.hideAll.connect(boost::bind(&ControlConnectBI::hide, this));
	ControlConnectBase::connect();
}

ControlConnectBD::ControlConnectBD(LyXView & lv, Dialogs & d)
	: ControlConnectBase(lv, d)
{}


void ControlConnectBD::connect()
{
	u_ = d_.updateBufferDependent.
		connect(boost::bind(&ControlConnectBD::updateSlot, this, _1));
	h_ = d_.hideBufferDependent.
		connect(boost::bind(&ControlConnectBD::hide, this));
	ControlConnectBase::connect();
}

void ControlConnectBD::disconnect()
{
	u_.disconnect();
	ControlConnectBase::disconnect();
}
