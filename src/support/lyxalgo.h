// -*- C++ -*-
/**
 * \file lyxalgo.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * A variety of useful templates.
 *
 * \author unknown
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


/// A slot in replacement for std::count for systems where it is broken.
template <class Iterator, class T>
typename std::iterator_traits<Iterator>::difference_type
count (Iterator first, Iterator last, T const & value)
{
#ifdef HAVE_STD_COUNT
	return std::count(first, last, value);
#else
	std::iterator_traits<Iterator>::difference_type n = 0;
	while (first != last)
		if (*first++ == value) ++n;
	return n;
#endif
}

/// Remove all duplicate entries in c.
template<class C>
void eliminate_duplicates(C & c)
{
	std::sort(c.begin(), c.end());
	typename C::iterator p = std::unique(c.begin(), c.end());
	c.erase(p, c.end());
}

} // namespace lyx

#endif // LYX_ALGO_H
