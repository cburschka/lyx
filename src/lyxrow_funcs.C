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


bool isParEnd(LyXText const & lt,
	ParagraphList::iterator pit, RowList::iterator rit)
{
	RowList::iterator next_row = boost::next(rit);
	return next_row == lt.rows().end() || lt.getPar(next_row) != pit;
}


pos_type lastPos(LyXText const & lt,
	ParagraphList::iterator pit, RowList::iterator rit)
{
	if (pit->empty())
		return 0;

	if (isParEnd(lt, pit, rit))
		return pit->size() - 1;

	return boost::next(rit)->pos() - 1;
}


namespace {

bool nextRowIsAllInset(
	ParagraphList::iterator pit, RowList::iterator rit, pos_type last)
{
	if (last + 1 >= pit->size())
		return false;

	if (!pit->isInset(last + 1))
		return false;

	InsetOld const * i = pit->getInset(last + 1);
	return i->needFullRow() || i->display();
}

} // anon namespace


pos_type lastPrintablePos(LyXText const & lt,
	ParagraphList::iterator pit, RowList::iterator rit)
{
	pos_type const last = lastPos(lt, pit, rit);

	// if this row is an end of par, just act like lastPos()
	if (isParEnd(lt, pit, rit))
		return last;

	if (!nextRowIsAllInset(pit, rit, last) && pit->isSeparator(last))
		return last - 1;

	return last;
}


int numberOfSeparators(LyXText const & lt,
	ParagraphList::iterator pit, RowList::iterator rit)
{
	pos_type const last = lastPrintablePos(lt, pit, rit);
	int n = 0;
	pos_type p = max(rit->pos(), pit->beginningOfBody());
	for ( ; p < last; ++p)
		if (pit->isSeparator(p))
			++n;
	return n;
}


// This is called _once_ from LyXText and should at least be moved into
// an anonymous namespace there. (Lgb)
int numberOfHfills(LyXText const & lt,
	ParagraphList::iterator pit, RowList::iterator rit)
{
	pos_type const last = lastPos(lt, pit, rit);
	pos_type first = rit->pos();

	// hfill *DO* count at the beginning of paragraphs!
	if (first) {
		while (first < last && pit->isHfill(first))
			++first;
	}

	first = max(first, pit->beginningOfBody());

	int n = 0;

	// last, because the end is ignored!
	for (pos_type p = first; p < last; ++p) {
		if (pit->isHfill(p))
			++n;
	}
	return n;
}


// This is called _once_ from LyXText and should at least be moved into
// an anonymous namespace there. (Lgb)
int numberOfLabelHfills(LyXText const & lt,
	ParagraphList::iterator pit, RowList::iterator rit)
{
	pos_type last = lastPos(lt, pit, rit);
	pos_type first = rit->pos();

	// hfill *DO* count at the beginning of paragraphs!
	if (first) {
		while (first < last && pit->isHfill(first))
			++first;
	}

	last = min(last, pit->beginningOfBody());
	int n = 0;

	// last, because the end is ignored!
	for (pos_type p = first; p < last; ++p) {
		if (pit->isHfill(p))
			++n;
	}
	return n;
}


bool hfillExpansion(LyXText const & lt,
	ParagraphList::iterator pit, RowList::iterator rit, pos_type pos)
{
	if (!pit->isHfill(pos))
		return false;

	// at the end of a row it does not count
	// unless another hfill exists on the line
	if (pos >= lastPos(lt, pit, rit))
		for (pos_type i = rit->pos(); i < pos && !pit->isHfill(i); ++i)
			return false;

	// at the beginning of a row it does not count, if it is not
	// the first row of a paragaph
	if (rit->isParStart())
		return true;

	// in some labels it does not count
	if (pit->layout()->margintype != MARGIN_MANUAL
	    && pos < pit->beginningOfBody())
		return false;

	// if there is anything between the first char of the row and
	// the specified position that is not a newline and not a hfill,
	// the hfill will count, otherwise not
	pos_type i = rit->pos();
	while (i < pos && (pit->isNewline(i) || pit->isHfill(i)))
		++i;

	return i != pos;
}
