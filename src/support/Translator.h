// -*- C++ -*-
/**
 * \file Translator.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "support/assert.h"

#include <vector>
#include <utility>


namespace lyx {

/**
 * This class template is used to translate between two elements, specifically
 * it was worked out to translate between an enum and strings when reading
 * the lyx file.
 *
 * The two template arguments should be of different types.
 */
template<typename T1, typename T2>
class Translator {
public:
	///
	typedef T1 first_argument_type;
	///
	typedef T2 second_argument_type;
	///
	typedef std::pair<T1, T2> MapPair;
	///
	typedef std::vector<MapPair> Map;
	///
	typedef typename Map::const_iterator const_iterator;

	///
	Translator(T1 const & t1, T2 const & t2)
		: default_t1(t1), default_t2(t2)
	{}

	/// Add a mapping to the translator.
	void addPair(T1 const & first, T2 const & second)
	{
		map.push_back(MapPair(first, second));
	}

	// Add the contents of \c other
	void add(Translator const & other)
	{
		if (other.map.empty())
			return;
		map.insert(map.end(), other.map.begin(), other.map.end());
	}

	/// Find the mapping for the first argument
	T2 const & find(T1 const & first) const
	{
		LASSERT(!map.empty(), /**/);
		const_iterator it = map.begin();
		const_iterator end = map.end();
		for (; it != end; ++it)
			if (it->first == first)
				return it->second;
		return default_t2;
	}

	/// Find the mapping for the second argument
	T1 const & find(T2 const & second) const
	{
		LASSERT(!map.empty(), /**/);
		const_iterator it = map.begin();
		const_iterator end = map.end();
		for (; it != end; ++it)
			if (it->second == second)
				return it->first;
		return default_t1;
	}
private:
	///
	Map map;
	///
	T1 const default_t1;
	///
	T2 const default_t2;
};


} // namespace lyx

#endif // TRANSLATOR_H
