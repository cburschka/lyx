/**
 * \file OutputParams.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 *  \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "OutputParams.h"
#include "Exporter.h"


namespace lyx {


OutputParams::OutputParams(Encoding const * enc)
	: flavor(LATEX), nice(false), moving_arg(false),
	  local_font(0), encoding(enc), free_spacing(false), use_babel(false),
	  linelen(0), depth(0),
	  exportdata(new ExportData),
	  inComment(false),
	  inDeletedInset(0), changeOfDeletedInset(Change::UNCHANGED),
	  par_begin(0), par_end(0),
	  dryrun(false)
{}


OutputParams::~OutputParams()
{}


} // namespace lyx
