/**
 * \file ControlConnections.C
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

#include "ControlConnections.h"

#include "ViewBase.h"

#include "buffer.h"
#include "BufferView.h"

#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"

#include <boost/bind.hpp>


ControlConnectBase::ControlConnectBase(LyXView & lv, Dialogs & d)
	: lv_(lv), d_(d)
{}


void ControlConnectBase::connect()
{
	r_ = d_.redrawGUI().
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


bool ControlConnectBase::bufferIsReadonly() const
{
	if (!lv_.buffer())
		return true;

	return lv_.buffer()->isReadonly();
}


bool ControlConnectBase::bufferIsAvailable() const
{
	if (!lv_.view())
		return false;

	return lv_.view()->available();
}


BufferView * ControlConnectBase::bufferview()
{
	return lv_.view().get();
}


BufferView const * ControlConnectBase::bufferview() const
{
	return lv_.view().get();
}


Buffer * ControlConnectBase::buffer()
{
	return lv_.buffer();
}


Buffer const * ControlConnectBase::buffer() const
{
	return lv_.buffer();
}


LyXFunc & ControlConnectBase::lyxfunc()
{
	return lv_.getLyXFunc();
}


LyXFunc const & ControlConnectBase::lyxfunc() const
{
	return lv_.getLyXFunc();
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
