#ifdef __GNUG__
#pragma implementation
#endif

#include "math_inset.h"
#include "math_charinset.h"
#include "math_scriptinset.h"
#include "debug.h"
#include "array.h"
#include "mathed/support.h"

using std::ostream;
using std::endl;


MathArray::MathArray()
{}


MathArray::MathArray(MathArray const & array, size_type from, size_type to)
	: bf_(array.begin() + from, array.begin() + to)
{}


void MathArray::substitute(MathMacro const & m)
{
	for (iterator it = begin(); it != end(); ++it)
		it->substitute(m);
}


MathAtom * MathArray::at(size_type pos)
{
	return pos < size() ? &bf_[pos] : 0;
}


MathAtom const * MathArray::at(size_type pos) const
{
	return pos < size() ? &bf_[pos] : 0;
}


void MathArray::insert(size_type pos, MathInset * p)
{
	//cerr << "\n  1: "; p->write(cerr, true); cerr << p << "\n";
	// inserting here invalidates the pointer!
	bf_.insert(begin() + pos, MathAtom(p));
	//cerr << "\n  2: "; p->write(cerr, true); cerr << p << "\n";
}


void MathArray::insert(size_type pos, MathArray const & array)
{
	bf_.insert(begin() + pos, array.begin(), array.end());
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
   

MathArray::size_type MathArray::size() const
{
	return bf_.size();
}


void MathArray::erase()
{
	erase(0, size());
}


void MathArray::erase(size_type pos)
{
	if (pos < size())
		erase(pos, pos + 1);
}


void MathArray::erase(size_type pos1, size_type pos2)
{
	bf_.erase(begin() + pos1, begin() + pos2);
}


MathAtom & MathArray::back()
{
	return bf_.back();
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
	MathCharInset const * p = it->nucleus()->asCharInset();
	if (!p)
		return s;

	for (MathTextCodes c = p->code(); it != end; ++it) {
		p = it->nucleus()->asCharInset();
		if (!p || it->up() || it->down() || p->code() != c)
			break;
		s += p->getChar();
	}
	return s;
}


void MathArray::write(ostream & os, bool fragile) const
{
	for (const_iterator it = begin(); it != end(); ++it) {
		MathCharInset const * p = it->nucleus()->asCharInset();
		if (p && !it->up() && !it->down()) {
			// special handling for character sequences with the same code
			string s = charSequence(it, end());
			p->writeHeader(os);
			os << s;
			p->writeTrailer(os);
			it += s.size() - 1;
		} else {
			it->write(os, fragile);
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
		it->validate(features);
}


void MathArray::pop_back()
{	
	if (!size()) {
		lyxerr << "pop_back from empty array!\n";
		return;
	}
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
