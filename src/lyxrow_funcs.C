/**
 * \file lyxrow_funcs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyxrow_funcs.h"
#include "debug.h"
#include "lyxlayout.h"
#include "lyxrow.h"
#include "paragraph.h"

using lyx::pos_type;


bool hfillExpansion(Paragraph const & par, Row const & row, pos_type pos)
{
	if (!par.isHfill(pos))
		return false;

	// at the end of a row it does not count
	// unless another hfill exists on the line
	if (pos >= row.endpos()) {
		for (pos_type i = row.pos(); i < pos && !par.isHfill(i); ++i)
			return false;
	}

	// at the beginning of a row it does not count, if it is not
	// the first row of a paragaph
	if (row.pos() == 0)
		return true;

	// in some labels it does not count
	if (par.layout()->margintype != MARGIN_MANUAL
	    && pos < par.beginOfBody())
		return false;

	// if there is anything between the first char of the row and
	// the specified position that is not a newline and not a hfill,
	// the hfill will count, otherwise not
	pos_type i = row.pos();
	while (i < pos && (par.isNewline(i) || par.isHfill(i)))
		++i;

	return i != pos;
}
