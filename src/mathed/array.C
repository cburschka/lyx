#ifdef __GNUG__
#pragma implementation
#endif

#include "math_inset.h"
#include "math_charinset.h"
#include "debug.h"
#include "array.h"
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
	deep_copy(begin(), end());
}


MathArray::MathArray(MathArray const & array, int from, int to)
	: bf_(array.begin() + from, array.begin() + to)
{
	deep_copy(begin(), end());
}


void MathArray::deep_copy(iterator from, iterator to)
{
	for (iterator it = from; it != to; ++it)
		*it = (*it)->clone();
}


int MathArray::last() const
{
	return size() - 1;
}


void MathArray::substitute(MathMacro const & m)
{
	MathArray tmp;
	for (iterator it = begin(); it != end(); ++it)
		(*it)->substitute(tmp, m);
	swap(tmp);
}


MathArray & MathArray::operator=(MathArray const & array)
{
	MathArray tmp(array);
	swap(tmp);
	return *this;
}


MathInset * MathArray::nextInset(int pos)
{
	return (pos == size()) ? 0 : bf_[pos];
}


MathInset const * MathArray::nextInset(int pos) const
{
	return (pos == size()) ? 0 : bf_[pos];
}


void MathArray::insert(int pos, MathInset * p)
{
	bf_.insert(begin() + pos, p);
}


void MathArray::insert(int pos, MathArray const & array)
{
	bf_.insert(begin() + pos, array.begin(), array.end());
	deep_copy(begin() + pos, begin() + pos + array.size());
}


void MathArray::push_back(MathInset * p)
{	
	insert(size(), p);
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
		erase(pos, pos + 1);
}


void MathArray::erase(int pos1, int pos2)
{
	for (iterator it = begin() + pos1; it != begin() + pos2; ++it)
		delete *it;
	bf_.erase(begin() + pos1, begin() + pos2);
}


MathInset * MathArray::back() const
{
	return size() ? bf_.back() : 0;
}


void MathArray::dump2(ostream & os) const
{
	for (const_iterator it = begin(); it != end(); ++it)
		os << *it << ' ';
}


void MathArray::dump(ostream & os) const
{
	for (const_iterator it = begin(); it != end(); ++it)
		os << "<" << *it << ">";
}


std::ostream & operator<<(std::ostream & os, MathArray const & ar)
{
	ar.dump2(os);
	return os;
}

// returns sequence of char with same code starting at it up to end
// it might be less, though...
string charSequence(MathArray::const_iterator it, MathArray::const_iterator end)
{
	string s;
	MathCharInset const * p = (*it)->asCharInset();
	if (!p)
		return s;

	MathTextCodes c = p->code();
	while (it != end && (p = (*it)->asCharInset()) && p->code() == c) { 
		s += p->getChar();
		++it;
	}
	return s;
}


void MathArray::write(ostream & os, bool fragile) const
{
	for (const_iterator it = begin(); it != end(); ) {
		MathCharInset const * p = (*it)->asCharInset();
		if (p) {
			// special handling for character sequences with the same code
			string s = charSequence(it, end());
			p->writeHeader(os);
			os << s;
			p->writeTrailer(os);
			it += s.size();
		} else {
			(*it)->write(os, fragile);
			++it;
		}
	}
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
	for (const_iterator it = begin(); it != end(); ++it)
		(*it)->validate(features);
}


void MathArray::pop_back()
{	
	if (!size()) {
		lyxerr << "pop_back from empty array!\n";
		return;
	}
	delete back();
	bf_.pop_back();
}


MathArray::const_iterator MathArray::begin() const
{
	return bf_.begin();
}


MathArray::const_iterator MathArray::end() const
{
	return bf_.end();
}


MathArray::iterator MathArray::begin()
{
	return bf_.begin();
}


MathArray::iterator MathArray::end()
{
	return bf_.end();
}
