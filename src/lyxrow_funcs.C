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


bool isParEnd(Paragraph const & par, Row const & row)
{
	return row.end() == par.size();
}


pos_type lastPos(Paragraph const & par, Row const & row)
{
	if (par.empty())
		return 0;
	pos_type pos = row.end() - 1;
	if (pos == par.size())
		--pos;
	return pos;
}


int numberOfSeparators(Paragraph const & par, Row const & row)
{
	pos_type const last = lastPos(par, row);
	int n = 0;
	pos_type p = max(row.pos(), par.beginningOfBody());
	for ( ; p < last; ++p)
		if (par.isSeparator(p))
			++n;
	return n;
}


// This is called _once_ from LyXText and should at least be moved into
// an anonymous namespace there. (Lgb)
int numberOfHfills(Paragraph const & par, Row const & row)
{
	pos_type const last = lastPos(par, row);
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
	pos_type last = lastPos(par, row);
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
	if (pos >= lastPos(par, row))
		for (pos_type i = row.pos(); i < pos && !par.isHfill(i); ++i)
			return false;

	// at the beginning of a row it does not count, if it is not
	// the first row of a paragaph
	if (row.isParStart())
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
