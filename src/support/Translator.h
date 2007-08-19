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

#include <boost/assert.hpp>
#include <boost/bind.hpp>

#include <vector>
#include <utility>
#include <algorithm>
#include <functional>


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
	Translator(T1 const & t1, T2 const & t2)
		: default_t1(t1), default_t2(t2)
		{}

	/// Add a mapping to the translator.
	void addPair(T1 const & first, T2 const & second) {
		map.push_back(MapPair(first, second));
	}
	// Add the contents of \c other
	void add(Translator const & other) {
		if (other.map.empty())
			return;
		map.insert(map.end(), other.map.begin(), other.map.end());
	}

	/// Find the mapping for the first argument
	T2 const & find(T1 const & first) const {
		BOOST_ASSERT(!map.empty());

		// For explanation see the next find() function.
		typename Map::const_iterator it =
			std::find_if(map.begin(), map.end(),
				     ::boost::bind(std::equal_to<T1>(),
						 ::boost::bind(&MapPair::first, _1),
						 first)
				);

		if (it != map.end()) {
			return it->second;
		} else {
			return default_t2;
		}
	}

	/// Find the mapping for the second argument
	T1 const & find(T2 const & second) const {
		BOOST_ASSERT(!map.empty());

		// The idea is as follows:
		// find_if() will try to compare the data in the vector with
		// the value. The vector is made of pairs and the value has
		// the type of the second part of the pair.
		// We thus give find_if() an equal_to functor and assign to
		// its second post the value we want to compare. We now
		// compose the equal_to functor with the select2nd functor
		// to take only the second value of the pair to be compared.
		//
		// We can depict it as follows:
		// equal_to(select2nd(pair) , second)
		typename Map::const_iterator it =
			std::find_if(map.begin(), map.end(),
				     ::boost::bind(std::equal_to<T2>(),
						 ::boost::bind(&MapPair::second, _1),
						 second)
				);

		if (it != map.end())
			return it->first;
		else {
			return default_t1;
		}
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
