/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright (C) 2000 The LyX Team.
 *
 *           @author Jürgen Vigna
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "GUIRunTime_pimpl.h"
#include <kapp.h>

using std::endl;

GUIRunTime::Pimpl::Pimpl()
{
}

GUIRunTime::Pimpl::~Pimpl() 
{
}

void GUIRunTime::Pimpl::processEvents() 
{
    kapp->processEvents();
}
