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
 * \file ControlUrl.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef CONTROLURL_H
#define CONTROLURL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlCommand.h"

/** A controller for the Url Dialog.
 */
class ControlUrl : public ControlCommand
{
public:
	///
	ControlUrl(LyXView &, Dialogs &);
};

#endif // CONTROLURL_H
