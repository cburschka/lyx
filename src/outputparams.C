/**
 * \file outputparams.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 *  \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "outputparams.h"
#include "exporter.h"


OutputParams::OutputParams()
	: flavor(LATEX), nice(false), moving_arg(false),
	  free_spacing(false), use_babel(false),
	  mixed_content(false), linelen(0), depth(0),
	  exportdata(new ExportData)
{}


OutputParams::~OutputParams()
{}
