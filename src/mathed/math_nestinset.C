#ifdef __GNUG__
#pragma implementation
#endif

#include "math_nestinset.h"
#include "math_mathmlstream.h"
#include "debug.h"


MathNestInset::MathNestInset(idx_type nargs)
	: MathDimInset(), cells_(nargs)
{}


MathInset::idx_type MathNestInset::nargs() const
{
	return cells_.size();
}


MathXArray & MathNestInset::xcell(idx_type i)
{
	return cells_[i];
}


MathXArray const & MathNestInset::xcell(idx_type i) const
{
	return cells_[i];
}


MathArray & MathNestInset::cell(idx_type i)
{
	return cells_[i].data_;
}


MathArray const & MathNestInset::cell(idx_type i) const
{
	return cells_[i].data_;
}


void MathNestInset::substitute(MathMacro const & m)
{
	for (idx_type i = 0; i < nargs(); ++i)
		cell(i).substitute(m);
}


void MathNestInset::metrics(MathMetricsInfo const & mi) const
{
	for (idx_type i = 0; i < nargs(); ++i)
		xcell(i).metrics(mi);
}


void MathNestInset::draw(Painter & pain, int x, int y) const
{
	for (idx_type i = 0; i < nargs(); ++i)
		xcell(i).draw(pain, x + xcell(i).xo(), y + xcell(i).yo());
}


bool MathNestInset::idxNext(idx_type & idx, pos_type & pos) const
{
	if (idx + 1 >= nargs())
		return false;
	++idx;
	pos = 0;
	return true;
}


bool MathNestInset::idxRight(idx_type & idx, pos_type & pos) const
{
	return idxNext(idx, pos);
}


bool MathNestInset::idxPrev(idx_type & idx, pos_type & pos) const
{
	if (idx == 0)
		return false;
	--idx;
	pos = cell(idx).size();
	return true;
}


bool MathNestInset::idxLeft(idx_type & idx, pos_type & pos) const
{
	return idxPrev(idx, pos);
}


bool MathNestInset::idxFirst(idx_type & i, pos_type & pos) const
{
	if (nargs() == 0)
		return false;
	i = 0;
	pos = 0;
	return true;
}


bool MathNestInset::idxLast(idx_type & i, pos_type & pos) const
{
	if (nargs() == 0)
		return false;
	i = nargs() - 1;
	pos = cell(i).size();
	return true;
}


bool MathNestInset::idxHome(idx_type & /* idx */, pos_type & pos) const
{
	if (pos == 0)
		return false;
	pos = 0;
	return true;
}


bool MathNestInset::idxEnd(idx_type & idx, pos_type & pos) const
{
	pos_type n = cell(idx).size();
	if (pos == n)
		return false;
	pos = n;
	return true;
}


void MathNestInset::dump() const
{
	WriteStream os(lyxerr);
	os << "---------------------------------------------\n";
	write(os);
	os << "\n";
	for (idx_type i = 0; i < nargs(); ++i)
		os << cell(i) << "\n";
	os << "---------------------------------------------\n";
}


void MathNestInset::validate(LaTeXFeatures & features) const
{
	for (idx_type i = 0; i < nargs(); ++i)
		cell(i).validate(features);
}


bool MathNestInset::match(MathInset * p) const
{
	if (nargs() != p->nargs())
		return false;
	for (idx_type i = 0; i < nargs(); ++i)
		if (!cell(i).match(p->cell(i)))
			return false;
	return true;
}


void MathNestInset::replace(ReplaceData & rep)
{
	for (idx_type i = 0; i < nargs(); ++i)
		cell(i).replace(rep);
}


bool MathNestInset::contains(MathArray const & ar)
{
	for (idx_type i = 0; i < nargs(); ++i)
		if (cell(i).contains(ar))
			return true;
	return false;
}
