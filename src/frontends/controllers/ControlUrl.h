// -*- C++ -*-
/**
 * \file ControlUrl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLURL_H
#define CONTROLURL_H


#include "ControlCommand.h"

/** A controller for the Url Dialog.
 */
class ControlUrl : public ControlCommand
{
public:
	///
	ControlUrl(Dialog &);
};

#endif // CONTROLURL_H
