/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 *           @author Jürgen Vigna
 *
 * ====================================================== */


#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "GUIRunTime.h"
#include "GUIRunTime_pimpl.h"

GUIRunTime::GUIRunTime()
{
	pimpl_ = new Pimpl();
}


GUIRunTime::~GUIRunTime()
{
	delete pimpl_;
}


void GUIRunTime::processEvents()
{
	pimpl_->processEvents();
}
