// -*- C++ -*-
/**
 * \file ControlIndex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLINDEX_H
#define CONTROLINDEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlCommand.h"

/** A controller for Index dialogs.
 */
class ControlIndex : public ControlCommand {
public:
	///
	ControlIndex(LyXView &, Dialogs &);
};

#endif // CONTROLINDEX_H
