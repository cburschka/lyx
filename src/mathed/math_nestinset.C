#ifdef __GNUG__
#pragma implementation
#endif

#include "math_nestinset.h"
#include "debug.h"


MathNestInset::MathNestInset(unsigned int nargs)
	: MathDimInset(), cells_(nargs)
{}


unsigned int MathNestInset::nargs() const
{
	return cells_.size();
}


MathXArray & MathNestInset::xcell(unsigned int i)
{
	return cells_[i];
}


MathXArray const & MathNestInset::xcell(unsigned int i) const
{
	return cells_[i];
}


MathArray & MathNestInset::cell(unsigned int i)
{
	return cells_[i].data_;
}


MathArray const & MathNestInset::cell(unsigned int i) const
{
	return cells_[i].data_;
}


void MathNestInset::substitute(MathMacro const & m)
{
	for (unsigned int i = 0; i < nargs(); ++i)
		cell(i).substitute(m);
}


void MathNestInset::metrics(MathStyles st) const
{
	size_ = st;
	for (unsigned int i = 0; i < nargs(); ++i)
		xcell(i).metrics(st);
}


void MathNestInset::draw(Painter & pain, int x, int y) const
{
	xo(x);
	yo(y);
	for (unsigned int i = 0; i < nargs(); ++i)
		xcell(i).draw(pain, x + xcell(i).xo(), y + xcell(i).yo());
}


bool MathNestInset::idxNext(unsigned int & idx, unsigned int & pos) const
{
	if (idx + 1 >= nargs())
		return false;
	++idx;
	pos = 0;
	return true;
}


bool MathNestInset::idxRight(unsigned int & idx, unsigned int & pos) const
{
	return idxNext(idx, pos);
}


bool MathNestInset::idxPrev(unsigned int & idx, unsigned int & pos) const
{
	if (idx == 0)
		return false;
	--idx;
	pos = cell(idx).size();
	return true;
}


bool MathNestInset::idxLeft(unsigned int & idx, unsigned int & pos) const
{
	return idxPrev(idx, pos);
}


bool MathNestInset::idxFirst(unsigned int & i, unsigned int & pos) const
{
	if (nargs() == 0)
		return false;
	i = 0;
	pos = 0;
	return true;
}


bool MathNestInset::idxLast(unsigned int & i, unsigned int & pos) const
{
	if (nargs() == 0)
		return false;
	i = nargs() - 1;
	pos = cell(i).size();
	return true;
}


bool MathNestInset::idxHome(unsigned int & /* idx */, unsigned int & pos) const
{
	if (pos == 0)
		return false;
	pos = 0;
	return true;
}


bool MathNestInset::idxEnd(unsigned int & idx, unsigned int & pos) const
{
	unsigned int n = cell(idx).size();
	if (pos == n)
		return false;

	pos = n;
	return true;
}


void MathNestInset::dump() const
{
	lyxerr << "---------------------------------------------\n";
	write(lyxerr, false);
	lyxerr << "\n";
	for (unsigned int i = 0; i < nargs(); ++i)
		lyxerr << cell(i) << "\n";
	lyxerr << "---------------------------------------------\n";
}


void MathNestInset::push_back(MathInset * p)
{
	if (nargs())
		cells_.back().data_.push_back(p);
	else
		lyxerr << "can't push without a cell\n";
}


void MathNestInset::validate(LaTeXFeatures & features) const
{
	for (unsigned int i = 0; i < nargs(); ++i)
		cell(i).validate(features);
}
