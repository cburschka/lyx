#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_data.h"
#include "math_inset.h"
#include "math_deliminset.h"
#include "math_charinset.h"
#include "math_scriptinset.h"
#include "math_stringinset.h"
#include "math_matrixinset.h"
#include "math_mathmlstream.h"
#include "math_support.h"
#include "math_replace.h"
#include "debug.h"
#include "support/LAssert.h"


MathArray::MathArray()
{}


MathArray::MathArray(MathArray const & ar, size_type from, size_type to)
	: bf_(ar.begin() + from, ar.begin() + to)
{}


MathArray::MathArray(iterator from, iterator to)
	: bf_(from, to)
{}


void MathArray::substitute(MathMacro const & m)
{
	for (iterator it = begin(); it != end(); ++it)
		it->nucleus()->substitute(m);
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


void MathArray::insert(size_type pos, MathArray const & ar)
{
	bf_.insert(begin() + pos, ar.begin(), ar.end());
}


void MathArray::push_back(MathAtom const & t)
{
	bf_.push_back(t);
}


void MathArray::push_back(MathArray const & ar)
{
	insert(size(), ar);
}


void MathArray::clear()
{
	erase();
}


void MathArray::swap(MathArray & ar)
{
	if (this != &ar)
		bf_.swap(ar.bf_);
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
	bf_.erase(begin(), end());
}


void MathArray::erase(size_type pos)
{
	if (pos < size())
		erase(pos, pos + 1);
}


void MathArray::erase(iterator pos1, iterator pos2)
{
	bf_.erase(pos1, pos2);
}


void MathArray::erase(iterator pos)
{
	bf_.erase(pos);
}


void MathArray::erase(size_type pos1, size_type pos2)
{
	bf_.erase(begin() + pos1, begin() + pos2);
}


MathAtom & MathArray::back()
{
	return bf_.back();
}


MathAtom & MathArray::front()
{
	return bf_.front();
}


MathAtom const & MathArray::front() const
{
	return bf_.front();
}


void MathArray::dump2() const
{
	NormalStream ns(lyxerr);
	for (const_iterator it = begin(); it != end(); ++it)
		ns << it->nucleus() << ' ';
}


void MathArray::dump() const
{
	NormalStream ns(lyxerr);
	for (const_iterator it = begin(); it != end(); ++it)
		ns << "<" << it->nucleus() << ">";
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


bool MathArray::match(MathArray const & ar) const
{
	return size() == ar.size() && matchpart(ar, 0);
}


bool MathArray::matchpart(MathArray const & ar, pos_type pos) const
{
	if (size() < ar.size() + pos)
		return false;
	const_iterator it = begin() + pos;
	for (const_iterator jt = ar.begin(); jt != ar.end(); ++jt, ++it)
		if (!jt->nucleus()->match(it->nucleus()))
			return false;
	return true;
}


void MathArray::replace(ReplaceData & rep)
{
	for (size_type i = 0; i < size(); ++i) {
		iterator it = begin() + i;
		const_iterator rt = rep.from.begin();
		const_iterator et = rep.from.end();
		for (const_iterator jt = it; jt != end() && rt != et; ++jt, ++rt)
			if (!jt->nucleus()->match(rt->nucleus()))
				break;
		if (rt == et) {
			// match found
			lyxerr << "match found!\n";
			erase(it, it + rep.from.size());
			insert(i, rep.to);
		}
	}

	for (const_iterator it = begin(); it != end(); ++it)
		it->nucleus()->replace(rep);
}


bool MathArray::contains(MathArray const & ar) const
{
	for (size_type i = 0; i + ar.size() <= size(); ++i) {
		const_iterator it = begin() + i;
		const_iterator rt = ar.begin();
		const_iterator et = ar.end();
		for (const_iterator jt = it; rt != et; ++jt, ++rt)
			if (!jt->nucleus()->match(rt->nucleus()))
				break;
		if (rt == et)
			return true;
	}

	for (const_iterator it = begin(); it != end(); ++it)
		if (it->nucleus()->contains(ar))
			return true;

	return false;
}
