/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlUrl.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlUrl.h"
#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "Dialogs.h"
#include "buffer.h"

#include "frontends/LyXView.h"

#include <boost/bind.hpp>

ControlUrl::ControlUrl(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d, LFUN_INSERT_URL)
{
	d_.showUrl = boost::bind(&ControlUrl::showInset, this, _1);
	d_.createUrl = boost::bind(&ControlUrl::createInset, this, _1);
}
