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


MathArray::MathArray(const_iterator from, const_iterator to)
	: base_type(from, to)
{}


void MathArray::substitute(MathMacro const & m)
{
	for (iterator it = begin(); it != end(); ++it)
		it->nucleus()->substitute(m);
}


MathAtom & MathArray::operator[](size_type pos)
{
	lyx::Assert(pos < size());
	return base_type::operator[](pos);
}


MathAtom const & MathArray::operator[](size_type pos) const
{
	lyx::Assert(pos < size());
	return base_type::operator[](pos);
}


void MathArray::insert(size_type pos, MathAtom const & t)
{
	base_type::insert(begin() + pos, t);
}


void MathArray::insert(size_type pos, MathArray const & ar)
{
	base_type::insert(begin() + pos, ar.begin(), ar.end());
}


void MathArray::append(MathArray const & ar)
{
	insert(size(), ar);
}


void MathArray::erase(size_type pos)
{
	if (pos < size())
		erase(pos, pos + 1);
}


void MathArray::erase(iterator pos1, iterator pos2)
{
	base_type::erase(pos1, pos2);
}


void MathArray::erase(iterator pos)
{
	base_type::erase(pos);
}


void MathArray::erase(size_type pos1, size_type pos2)
{
	base_type::erase(begin() + pos1, begin() + pos2);
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
		if (find1(rep.from, i)) {
			// match found
			lyxerr << "match found!\n";
			erase(i, i + rep.from.size());
			insert(i, rep.to);
		}
	}

	for (const_iterator it = begin(); it != end(); ++it)
		it->nucleus()->replace(rep);
}


bool MathArray::find1(MathArray const & ar, size_type pos) const
{
	//lyxerr << "finding '" << ar << "' in '" << *this << "'\n";
	for (size_type i = 0, n = ar.size(); i < n; ++i)
		if (!operator[](pos + i)->match(ar[i].nucleus()))
			return false;
	return true;
}


MathArray::size_type MathArray::find(MathArray const & ar) const
{
	for (int i = 0, last = size() - ar.size(); i < last; ++i)
		if (find1(ar, i))
			return i;
	return size();
}


MathArray::size_type MathArray::find_last(MathArray const & ar) const
{
	for (int i = size() - ar.size(); i >= 0; --i) 
		if (find1(ar, i))
			return i;
	return size();
}


bool MathArray::contains(MathArray const & ar) const
{
	if (find(ar) != size())
		return true;
	for (const_iterator it = begin(); it != end(); ++it)
		if (it->nucleus()->contains(ar))
			return true;
	return false;
}
