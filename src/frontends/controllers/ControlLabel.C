/**
 * \file ControlLabel.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "ControlLabel.h"


ControlLabel::ControlLabel(Dialog & d)
	: ControlCommand(d, "label")
{}
