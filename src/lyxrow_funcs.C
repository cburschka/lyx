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

using std::max;
using std::min;
using std::endl;


int numberOfSeparators(Paragraph const & par, Row const & row)
{
	pos_type const first = max(row.pos(), par.beginningOfBody());
	pos_type const last = row.endpos() - 1;
	int n = 0;
	for (pos_type p = first; p < last; ++p) {
		if (par.isSeparator(p))
			++n;
	}

	return n;
}


// This is called _once_ from LyXText and should at least be moved into
// an anonymous namespace there. (Lgb)
int numberOfHfills(Paragraph const & par, Row const & row)
{
	pos_type const last = row.endpos() - 1;
	pos_type first = row.pos();

	// hfill *DO* count at the beginning of paragraphs!
	if (first) {
		while (first < last && par.isHfill(first))
			++first;
	}

	first = max(first, par.beginningOfBody());

	int n = 0;

	// last, because the end is ignored!
	for (pos_type p = first; p < last; ++p) {
		if (par.isHfill(p))
			++n;
	}

	return n;
}


// This is called _once_ from LyXText and should at least be moved into
// an anonymous namespace there. (Lgb)
int numberOfLabelHfills(Paragraph const & par, Row const & row)
{
	pos_type last = row.endpos() - 1;
	pos_type first = row.pos();

	// hfill *DO* count at the beginning of paragraphs!
	if (first) {
		while (first < last && par.isHfill(first))
			++first;
	}

	last = min(last, par.beginningOfBody());
	int n = 0;

	// last, because the end is ignored
	for (pos_type p = first; p < last; ++p) {
		if (par.isHfill(p))
			++n;
	}

	return n;
}


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
	    && pos < par.beginningOfBody())
		return false;

	// if there is anything between the first char of the row and
	// the specified position that is not a newline and not a hfill,
	// the hfill will count, otherwise not
	pos_type i = row.pos();
	while (i < pos && (par.isNewline(i) || par.isHfill(i)))
		++i;

	return i != pos;
}
