// -*- C++ -*-
/**
 * \file ControlLabel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLLABEL_H
#define CONTROLLABEL_H


#include "ControlCommand.h"

/** A controller for Label dialogs.
 */
class ControlLabel : public ControlCommand {
public:
	///
	ControlLabel(Dialog &);
};

#endif // CONTROLLABEL_H
