// -*- C++ -*-

#ifndef LYX_ALGO_H
#define LYX_ALGO_H

// Both these functions should ideally be placed into namespace lyx.
// Also the using std::less should not be used.

//namespace lyx {

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

// }  // end of namespace lyx
#endif
