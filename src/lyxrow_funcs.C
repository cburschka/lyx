#include <config.h>

#include "lyxrow_funcs.h"
#include "lyxtext.h"
#include "paragraph.h"
#include "lyxlayout.h"

#include <boost/next_prior.hpp>
#include <algorithm>

using lyx::pos_type;

using std::max;
using std::min;


bool isParEnd(Paragraph const & par, RowList::iterator rit)
{
	return boost::next(rit) == par.rows.end();
}


pos_type lastPos(Paragraph const & par, RowList::iterator rit)
{
	if (par.empty())
		return 0;

	if (isParEnd(par, rit))
		return par.size() - 1;

	return boost::next(rit)->pos() - 1;
}


namespace {

bool nextRowIsAllInset(
	Paragraph const & par, pos_type last)
{
	if (last + 1 >= par.size())
		return false;

	if (!par.isInset(last + 1))
		return false;

	InsetOld const * i = par.getInset(last + 1);
	return i->needFullRow() || i->display();
}

} // anon namespace


pos_type lastPrintablePos(Paragraph const & par, RowList::iterator rit)
{
	pos_type const last = lastPos(par, rit);

	// if this row is an end of par, just act like lastPos()
	if (isParEnd(par, rit))
		return last;

	if (!nextRowIsAllInset(par, last) && par.isSeparator(last))
		return last - 1;

	return last;
}


int numberOfSeparators(Paragraph const & par, RowList::iterator rit)
{
	pos_type const last = lastPrintablePos(par, rit);
	int n = 0;
	pos_type p = max(rit->pos(), par.beginningOfBody());
	for ( ; p < last; ++p)
		if (par.isSeparator(p))
			++n;
	return n;
}


// This is called _once_ from LyXText and should at least be moved into
// an anonymous namespace there. (Lgb)
int numberOfHfills(Paragraph const & par, RowList::iterator rit)
{
	pos_type const last = lastPos(par, rit);
	pos_type first = rit->pos();

	// hfill *DO* count at the beginning of paragraphs!
	if (first) {
		while (first < last && par.isHfill(first))
			++first;
	}

	first = max(first, par.beginningOfBody());

	int n = 0;

	// last, because the end is ignored!
	for (pos_type p = first; p < last; ++p)
		if (par.isHfill(p))
			++n;

	return n;
}


// This is called _once_ from LyXText and should at least be moved into
// an anonymous namespace there. (Lgb)
int numberOfLabelHfills(Paragraph const & par, RowList::iterator rit)
{
	pos_type last = lastPos(par, rit);
	pos_type first = rit->pos();

	// hfill *DO* count at the beginning of paragraphs!
	if (first) {
		while (first < last && par.isHfill(first))
			++first;
	}

	last = min(last, par.beginningOfBody());
	int n = 0;

	// last, because the end is ignored!
	for (pos_type p = first; p < last; ++p) {
		if (par.isHfill(p))
			++n;
	}
	return n;
}


bool hfillExpansion(Paragraph const & par, RowList::iterator rit, pos_type pos)
{
	if (!par.isHfill(pos))
		return false;

	// at the end of a row it does not count
	// unless another hfill exists on the line
	if (pos >= lastPos(par, rit))
		for (pos_type i = rit->pos(); i < pos && !par.isHfill(i); ++i)
			return false;

	// at the beginning of a row it does not count, if it is not
	// the first row of a paragaph
	if (rit->isParStart())
		return true;

	// in some labels it does not count
	if (par.layout()->margintype != MARGIN_MANUAL
	    && pos < par.beginningOfBody())
		return false;

	// if there is anything between the first char of the row and
	// the specified position that is not a newline and not a hfill,
	// the hfill will count, otherwise not
	pos_type i = rit->pos();
	while (i < pos && (par.isNewline(i) || par.isHfill(i)))
		++i;

	return i != pos;
}
