// -*- C++ -*-
/**
 * \file lyxalgo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 *
 * A variety of useful templates.
 */

#ifndef LYX_ALGO_H
#define LYX_ALGO_H

#include <utility>
#include <iterator>
#include <algorithm>


namespace lyx {


/// Returns true if the sequence first,last is sorted, false if not.
template <class For>
bool sorted(For first, For last)
{
	if (first == last) return true;
	For tmp = first;
	while (++tmp != last) {
		if (*tmp < *first++) return false;
	}
	return true;
}


/// Cmp is the same Cmp as you would pass to std::sort.
template <class For, class Cmp>
bool sorted(For first, For last, Cmp cmp)
{
	if (first == last) return true;
	For tmp = first;
	while (++tmp != last) {
		if (cmp(*tmp, *first++)) return false;
	}
	return true;
}


struct firster {
	template <class P1, class P2>
	P1 operator()(std::pair<P1, P2> const & p) {
		return p.first;
	}
};


/**
 * copy elements in the given range to the output iterator
 * if the predicate evaluates as true
 */
template <class InputIter, class OutputIter, class Func>
OutputIter copy_if(InputIter first, InputIter last,
	       OutputIter result, Func func)
{
	for (; first != last; ++first) {
		if (func(*first)) {
			*result++ = *first;
		}
	}
	return result;
}


/// Remove all duplicate entries in c.
template<class C>
void eliminate_duplicates(C & c)
{
	// It is a requirement that the container is sorted for
	// std::unique to work properly.
	std::sort(c.begin(), c.end());
	c.erase(std::unique(c.begin(), c.end()), c.end());
}


#ifdef LYX_USE_CXX11
using std::next;
#else
/// Replacement of std::next for older compilers
template <typename It, typename Diff>
inline It next(It i, Diff n = 1)
{
	std::advance(i, n);
	return i;
}
#endif


#ifdef LYX_USE_CXX11
using std::prev;
#else
/// Replacement of std::prev for older compilers
template <typename It, typename Diff>
inline It prev(It i, Diff n = 1)
{
	std::advance(i, -n);
	return i;
}
#endif

} // namespace lyx

#endif // LYX_ALGO_H
