#ifndef COORDCACHE_H
#define COORDCACHE_H

#include "mathed/math_data.h"
#include "insets/insetbase.h"
#include "lyxtext.h"

#include <boost/assert.hpp>

#include <map>


// All positions cached in this cache are only valid between subsequent
// updated. (x,y) == (0,0) is the upper left screen corner, x increases
// to the right, y increases downwords.

void lyxbreaker(void const * data, const char * hint, int size);

struct Point {
	Point()
		: x_(0), y_(0)
	{}	

	Point(int x, int y) : x_(x), y_(y)
	{
		BOOST_ASSERT(x > -3000);
		BOOST_ASSERT(x <  4000);
		BOOST_ASSERT(y > -3000);
		BOOST_ASSERT(y <  4000);
	}	

	int x_, y_;
};


template <class T> class CoordCacheBase {
public:
	void clear()
	{
		data_.clear();
	}

	void add(T const * thing, int x, int y)
	{
		data_[thing] = Point(x, y);
	}

	int x(T const * thing) const
	{
		check(thing, "x");
		return data_.find(thing)->second.x_;
	}

	int y(T const * thing) const
	{
		check(thing, "y");
		return data_.find(thing)->second.y_;
	}

	Point xy(T const * thing) const
	{
		check(thing, "xy");
		return data_.find(thing)->second;
	}

	bool has(T const * thing) const
	{
		return data_.find(thing) != data_.end();
	}

//	T * find(int x, int y) const
//	{
//		T * 
//		cache_type iter
//	} 

private:
	friend class CoordCache;

	void check(T const * thing, char const * hint) const
	{
		if (!has(thing)) {
			lyxbreaker(thing, hint, data_.size());
			BOOST_ASSERT(false);
		}
	}

	typedef std::map<T const *, Point> cache_type;
	cache_type data_;
};


class CoordCache {
public:
	void clear();

	CoordCacheBase<MathArray> arrays_;
	CoordCacheBase<InsetBase> insets_;
};

extern CoordCache theCoords;

#endif
