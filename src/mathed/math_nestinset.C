#ifdef __GNUG__
#pragma implementation
#endif

#include "math_nestinset.h"
#include "debug.h"


MathNestInset::MathNestInset(int nargs)
	: MathDimInset(), cells_(nargs)
{}


int MathNestInset::nargs() const
{
	return cells_.size();
}


MathXArray & MathNestInset::xcell(int i)
{
	return cells_[i];
}


MathXArray const & MathNestInset::xcell(int i) const
{
	return cells_[i];
}


MathArray & MathNestInset::cell(int i)
{
	return cells_[i].data_;
}


MathArray const & MathNestInset::cell(int i) const
{
	return cells_[i].data_;
}


void MathNestInset::substitute(MathArray & array, MathMacro const & m) const
{
#warning Huch?
/*
	MathNestInset * p = clone();
	array.push_back(clone());
	for (int i = 0; i < nargs(); ++i)
		array.back().cellsubstitute(m);
*/
}


void MathNestInset::metrics(MathStyles st) const
{
	size_ = st;
	for (int i = 0; i < nargs(); ++i)
		xcell(i).metrics(st);
}


void MathNestInset::draw(Painter & pain, int x, int y) const
{
	xo(x);
	yo(y);
	for (int i = 0; i < nargs(); ++i)
		xcell(i).draw(pain, x + xcell(i).xo(), y + xcell(i).yo());
}


bool MathNestInset::idxNext(int & idx, int & pos) const
{
	if (idx + 1 >= nargs())
		return false;
	++idx;
	pos = 0;
	return true;
}


bool MathNestInset::idxRight(int & idx, int & pos) const
{
	return idxNext(idx, pos);
}


bool MathNestInset::idxPrev(int & idx, int & pos) const
{
	if (idx == 0)
		return false;
	--idx;
	pos = cell(idx).size();
	return true;
}


bool MathNestInset::idxLeft(int & idx, int & pos) const
{
	return idxPrev(idx, pos);
}


bool MathNestInset::idxFirst(int & i, int & pos) const
{
	if (nargs() == 0)
		return false;
	i = 0;
	pos = 0;
	return true;
}


bool MathNestInset::idxLast(int & i, int & pos) const
{
	if (nargs() == 0)
		return false;
	i = nargs() - 1;
	pos = cell(i).size();
	return true;
}


bool MathNestInset::idxHome(int & /* idx */, int & pos) const
{
	if (pos == 0)
		return false;
	pos = 0;
	return true;
}


bool MathNestInset::idxEnd(int & idx, int & pos) const
{
	if (pos == cell(idx).size())
		return false;

	pos = cell(idx).size();
	return true;
}


void MathNestInset::dump() const
{
	lyxerr << "---------------------------------------------\n";
	write(lyxerr, false);
	lyxerr << "\n";
	for (int i = 0; i < nargs(); ++i)
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
	for (int i = 0; i < nargs(); ++i)
		cell(i).validate(features);
}
