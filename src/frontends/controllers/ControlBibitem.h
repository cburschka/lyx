// -*- C++ -*-
/**
 * \file ControlBibitem.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLBIBITEM_H
#define CONTROLBIBITEM_H


#include "ControlCommand.h"

/** A controller for Bibitem dialogs.
 */
class ControlBibitem : public ControlCommand {
public:
	///
	ControlBibitem(Dialog &);
};

#endif // CONTROLBIBITEM_H
