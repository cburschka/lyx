/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlIndex.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
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
