#ifdef __GNUG__
#pragma implementation
#endif

#include "math_inset.h"
#include "math_charinset.h"
#include "debug.h"
#include "array.h"
#include "math_scriptinset.h"
#include "mathed/support.h"

using std::ostream;
using std::endl;

MathArray::MathArray()
{}


MathArray::~MathArray()
{
	erase();
}


MathArray::MathArray(MathArray const & array)
	: bf_(array.bf_)
{
	deep_copy(0, size());
}


MathArray::MathArray(MathArray const & array, int from, int to)
	: bf_(array.bf_.begin() + from, array.bf_.begin() + to)
{
	deep_copy(0, size());
}


void MathArray::deep_copy(int pos1, int pos2)
{
	for (int pos = pos1; pos < pos2; ++pos) 
		bf_[pos] = bf_[pos]->clone();
}


bool MathArray::next(int & pos) const
{
	if (pos >= size() - 1)
		return false;

	++pos;
	return true;
}


bool MathArray::prev(int & pos) const
{
	if (pos == 0)
		return false;

	--pos;
	return true;
}


bool MathArray::last(int & pos) const
{
	pos = bf_.size();
	return prev(pos);
}


void MathArray::substitute(MathMacro const & m)
{
	MathArray tmp;
	for (int pos = 0; pos < size(); ++pos) 
		bf_[pos]->substitute(tmp, m);
	swap(tmp);
}


MathArray & MathArray::operator=(MathArray const & array)
{
	MathArray tmp(array);
	swap(tmp);
	return *this;
}


MathInset * MathArray::nextInset(int pos) const
{
	return (pos == size()) ? 0 : bf_[pos];
}


MathInset * MathArray::prevInset(int pos) const
{
	return (pos == 0) ? 0 : bf_[pos - 1];
}


unsigned char MathArray::getChar(int pos) const
{
	return (pos == size()) ? 0 : (bf_[pos]->getChar());
}


/*
string MathArray::getString(int & pos) const
{
	string s;
	if (isInset(pos))
		return s;

	MathTextCodes const fcode = getCode(pos);
	do {
		s += getChar(pos);
		next(pos);
	} while (pos < size() && !isInset(pos) && getCode(pos) == fcode);

	return s;
}
*/


MathTextCodes MathArray::getCode(int pos) const
{
	return pos < size() ? (bf_[pos]->code()) : LM_TC_MIN;
}


void MathArray::setCode(int pos, MathTextCodes t)
{
	bf_[pos]->code(t);
}


void MathArray::insert(int pos, MathInset * p)
{
	bf_.insert(bf_.begin() + pos, p);
}


void MathArray::insert(int pos, unsigned char b, MathTextCodes t)
{
	bf_.insert(bf_.begin() + pos, new MathCharInset(b, t));
}


void MathArray::insert(int pos, MathArray const & array)
{
	bf_.insert(bf_.begin() + pos, array.bf_.begin(), array.bf_.end());
	deep_copy(pos, pos + array.size());
}


void MathArray::push_back(MathInset * p)
{	
	insert(size(), p);
}


void MathArray::push_back(unsigned char b, MathTextCodes c)
{
	insert(size(), b, c);
}


void MathArray::push_back(MathArray const & array)
{
	insert(size(), array);
}


void MathArray::clear()
{
	erase();
}


void MathArray::swap(MathArray & array)
{
	if (this != &array) 
		bf_.swap(array.bf_);
}


bool MathArray::empty() const
{
	return bf_.empty();
}
   

int MathArray::size() const
{
	return bf_.size();
}


void MathArray::erase()
{
	erase(0, size());
}


void MathArray::erase(int pos)
{
	if (pos < size())
		bf_.erase(bf_.begin() + pos);
}


void MathArray::erase(int pos1, int pos2)
{
	for (int pos = pos1; pos < pos2; ++pos)
		delete nextInset(pos);
	bf_.erase(bf_.begin() + pos1, bf_.begin() + pos2);
}


MathInset * MathArray::back() const
{
	return prevInset(size());
}


void MathArray::dump2(ostream & os) const
{
	for (buffer_type::const_iterator it = bf_.begin(); it != bf_.end(); ++it)
		os << int(*it) << ' ';
	os << endl;
}


void MathArray::dump(ostream & os) const
{
	for (int pos = 0; pos < size(); ++pos)
		os << "<" << nextInset(pos) << ">";
}


std::ostream & operator<<(std::ostream & os, MathArray const & ar)
{
	ar.dump2(os);
	return os;
}


void MathArray::write(ostream & os, bool fragile) const
{
	for (int pos = 0; pos < size(); ++pos)
		nextInset(pos)->write(os, fragile);
}


void MathArray::writeNormal(ostream & os) const
{
	if (empty()) {
		os << "[par] ";
		return;
	}

	write(os, true);
}


void MathArray::validate(LaTeXFeatures & features) const
{
	for (int pos = 0; pos < size(); ++pos)
		nextInset(pos)->validate(features);
}


void MathArray::pop_back()
{	
	int pos = size();
	prev(pos);
	erase(pos, size());
}

