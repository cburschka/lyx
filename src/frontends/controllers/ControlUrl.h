// -*- C++ -*-
/**
 * \file ControlUrl.h
 * See the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
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
