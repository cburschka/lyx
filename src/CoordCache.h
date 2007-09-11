// -*- C++ -*-
/* \file CoordCache.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef COORDCACHE_H
#define COORDCACHE_H

// It seems that MacOSX define the check macro.
#undef check

#include "support/types.h"

#include <map>

namespace lyx {

class Inset;
class Text;
class MathData;
class Paragraph;

void lyxbreaker(void const * data, const char * hint, int size);

class Point {
public:
	Point()
		: x_(0), y_(0)
	{}

	Point(int x, int y);

	int x_, y_;
};

template <class T> class CoordCacheBase {
public:
	void clear()
	{
		data_.clear();
	}

	bool const empty() const
	{
		return data_.empty();
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
		if (!has(thing))
			lyxbreaker(thing, hint, data_.size());
	}

	typedef std::map<T const *, Point> cache_type;
	cache_type data_;

public:
	cache_type const & getData() const { return data_; }
};

/**
 * A global cache that allows us to come from a paragraph in a document
 * to a position point on the screen.
 * All points cached in this cache are only valid between subsequent
 * updates. (x,y) == (0,0) is the upper left screen corner, x increases
 * to the right, y increases downwords.
 * The cache is built in BufferView::updateMetrics which is called
 * from BufferView::update. The individual points are added
 * while we paint them. See for instance paintPar in RowPainter.C.
 */
class CoordCache {
public:
	void clear();

	/// A map from paragraph index number to screen point
	typedef std::map<pit_type, Point> InnerParPosCache;
	/// A map from a CursorSlice to screen points
	typedef std::map<Text const *, InnerParPosCache> SliceCache;

	/// A map from MathData to position on the screen
	CoordCacheBase<MathData> & arrays() { return arrays_; }
	CoordCacheBase<MathData> const & getArrays() const { return arrays_; }
	/// A map from insets to positions on the screen
	CoordCacheBase<Inset> & insets() { return insets_; }
	CoordCacheBase<Inset> const & getInsets() const { return insets_; }
	///
	SliceCache & slice(bool boundary)
	{
		return boundary ? slices1_ : slices0_;
	}
	SliceCache const & getSlice(bool boundary) const
	{
		return boundary ? slices1_ : slices0_;
	}

	/// Dump the contents of the cache to lyxerr in debugging form
	void dump() const;
private:
	/// MathDatas
	CoordCacheBase<MathData> arrays_;
	// All insets
	CoordCacheBase<Inset> insets_;
	/// Used with boundary == 0
	SliceCache slices0_;
	/// Used with boundary == 1
	SliceCache slices1_;
};

} // namespace lyx

#endif
