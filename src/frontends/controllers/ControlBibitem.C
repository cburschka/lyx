/**
 * \file ControlBibitem.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlBibitem.h"


ControlBibitem::ControlBibitem(Dialog & d)
	: ControlCommand(d, "bibitem")
{}
