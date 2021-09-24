// -*- C++ -*-
/**
 * \file Cache.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CACHE_H
#define CACHE_H

#include <QCache>

#include <list>
#include <type_traits>

namespace lyx {

/**
 * Cache<Key, T> implements a cache where objects are stored by copy.
 *
 * This is a wrapper for QCache. See the documentation of QCache:
 * <https://doc.qt.io/qt-5/qcache.html>.
 *
 * It is especially useful for storing shared pointers. This turns QCache into a
 * shared-ownership cache with no risks of dangling pointer. It is also useful
 * for small copyable objects.
 *
 * Use this rather than QCache directly, to avoid naked pointers.
 */
template <class Key, class Val>
class Cache : private QCache<Key, Val> {
	static_assert(std::is_copy_constructible<Val>::value,
	              "lyx::Cache only stores copyable objects!");
	static_assert(std::is_default_constructible<Val>::value,
	              "lyx::Cache only stores default-constructible objects!");
	using Q = QCache<Key, Val>;

public:
	///
	Cache(int max_cost = 100) : Q(max_cost) {}
	///
	bool insert(Key const & key, Val object, int cost = 1)
	{
		return Q::insert(key, new Val(std::move(object)), cost);
	}
	// Returns the default value (e.g. null pointer) if not found in the
	// cache. If this is not convenient for your class Val, check if it exists
	// beforehand with Cache::contains.
	Val object(Key const & key) const
	{
		if (Val * obj = Q::object(key))
			return *obj;
		return Val();
	}
	// Version that returns a pointer, and nullptr if the object is not present
	Val * object_ptr(Key const & key) const { return Q::object(key); }
	/// Synonymous for object, same remark as above.
	Val operator[](Key const & key) const { return object(key); }
	/// Everything from QCache except QCache::take.
	using Q::clear;
	using Q::contains;
	using Q::count;
	using Q::remove;
	using Q::size;
	bool empty() const { return Q::isEmpty(); }
	std::list<Key> keys() { return Q::keys().toStdList(); }
	int max_cost() const { return Q::maxCost(); }
	void set_max_cost(int cost) { Q::setMaxCost(cost); }
	int total_cost() const { return Q::totalCost(); }
};

} // namespace lyx

#endif
