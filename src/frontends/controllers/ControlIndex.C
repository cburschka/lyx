/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlIndex.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlIndex.h"
#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "buffer.h"
#include "Dialogs.h"

#include "frontends/LyXView.h"

#include <boost/bind.hpp>

ControlIndex::ControlIndex(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d, LFUN_INDEX_INSERT)
{
	d_.showIndex = boost::bind(&ControlIndex::showInset, this, _1);
	d_.createIndex = boost::bind(&ControlIndex::createInset, this, _1);
}
