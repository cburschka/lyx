#ifdef __GNUG__
#pragma implementation
#endif

#include "math_inset.h"
#include "math_charinset.h"
#include "math_scriptinset.h"
#include "math_stringinset.h"
#include "debug.h"
#include "array.h"
#include "mathed/support.h"
#include "support/LAssert.h"

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
		it->nucleus()->substitute(m);
}


MathScriptInset const * MathArray::asScript(const_iterator it) const
{
	if (it->nucleus()->asScriptInset())
		return 0;
	const_iterator jt = it + 1;
	if (jt == end())
		return 0;
	if (!jt->nucleus())
		return 0;
	return jt->nucleus()->asScriptInset();
}


MathAtom & MathArray::at(size_type pos)
{
	lyx::Assert(pos < size());
	return bf_[pos];
}


MathAtom const & MathArray::at(size_type pos) const
{
	lyx::Assert(pos < size());
	return bf_[pos];
}


void MathArray::insert(size_type pos, MathAtom const & t)
{
	bf_.insert(begin() + pos, t);
}


void MathArray::insert(size_type pos, MathArray const & array)
{
	bf_.insert(begin() + pos, array.begin(), array.end());
}


void MathArray::push_back(MathAtom const & t)
{	
	bf_.push_back(t);
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
		os << it->nucleus() << ' ';
}


void MathArray::dump(ostream & os) const
{
	for (const_iterator it = begin(); it != end(); ++it)
		os << "<" << it->nucleus() << ">";
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
		if (!it->nucleus())
			break;
		p = it->nucleus()->asCharInset();
		if (!p || p->code() != c)
			break;
		s += p->getChar();
	}
	return s;
}


MathArray MathArray::glueChars() const
{
	MathArray ar;
	const_iterator it = begin();
	while (it != end()) {
		if (it->nucleus() && it->nucleus()->asCharInset()) {
			string s = charSequence(it, end());
			MathTextCodes c = it->nucleus()->asCharInset()->code();
			ar.push_back(MathAtom(new MathStringInset(s, c)));
			it += s.size();
		} else {
			ar.push_back(*it);
			++it;
		}
	}
	return ar;
}


bool needAsterisk(MathAtom const & a, MathAtom const & b)
{
	return false;
}


MathArray MathArray::guessAsterisks() const
{
	if (size() <= 1)
		return *this;
	MathArray ar;
	ar.push_back(*begin());
	for (const_iterator it = begin(), jt = begin()+1 ; jt != end(); ++it, ++jt) {
		if (needAsterisk(*it, *jt))
			ar.push_back(MathAtom(new MathCharInset('*')));
		ar.push_back(*it);
	}
	ar.push_back(*end());
	return ar;
}


void MathArray::write(MathWriteInfo & wi) const
{
	MathArray ar = glueChars();
	for (const_iterator it = ar.begin(); it != ar.end(); ++it) {
		MathInset const * p = it->nucleus();
		if (MathScriptInset const * q = ar.asScript(it)) {
			q->write(p, wi);
			++it;
		} else {
			p->write(wi);
		}
	}
}


void MathArray::writeNormal(ostream & os) const
{
	MathArray ar = glueChars();
	for (const_iterator it = ar.begin(); it != ar.end(); ++it) {
		MathInset const * p = it->nucleus();
		if (MathScriptInset const * q = ar.asScript(it)) {
			q->writeNormal(p, os);
			++it;
		} else 
			p->writeNormal(os);
	}
}


string MathArray::octavize() const
{
	MathArray ar = glueChars();
	string res;
	for (const_iterator it = ar.begin(); it != ar.end(); ++it) {
		MathInset const * p = it->nucleus();
		if (MathScriptInset const * q = ar.asScript(it)) {
			res += q->octavize(p);
			++it;
		} else 
			res += p->octavize();
	}
	return res;
}


string MathArray::maplize() const
{
	MathArray ar = glueChars();
	string res;
	for (const_iterator it = ar.begin(); it != ar.end(); ++it) {
		MathInset const * p = it->nucleus();
		if (MathScriptInset const * q = ar.asScript(it)) {
			res += q->maplize(p);
			++it;
		} else 
			res += p->maplize();
	}
	return res;
}


string MathArray::mathmlize() const
{
	MathArray ar = glueChars();
	string res;
	for (const_iterator it = ar.begin(); it != ar.end(); ++it) {
		MathInset const * p = it->nucleus();
		if (MathScriptInset const * q = ar.asScript(it)) {
			res += q->mathmlize(p);
			++it;
		} else 
			res += p->mathmlize();
	}
	return res;
}


void MathArray::validate(LaTeXFeatures & features) const
{
	for (const_iterator it = begin(); it != end(); ++it)
		if (it->nucleus())
			it->nucleus()->validate(features);
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


