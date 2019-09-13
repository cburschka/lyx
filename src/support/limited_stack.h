// -*- C++ -*-
/**
 * \file limited_stack.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LIMITED_STACK_H
#define LIMITED_STACK_H

#include <deque>


namespace lyx {

/**
 * limited_stack - A stack of limited size.
 *
 * Like a normal stack, but elements falling out
 * of the bottom are destructed.
 */
template <typename T>
class limited_stack {
public:
	typedef std::deque<T> container_type;
	typedef typename container_type::value_type value_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::const_iterator const_iterator;

	/// limit is the maximum size of the stack
	limited_stack(size_type limit = 100) : limit_(limit) {}

	/// Return the top element.
	value_type & top() {
		return c_.front();
	}

	/// Pop and throw away the top element.
	void pop() {
		c_.pop_front();
	}

	/// Return true if the stack is empty.
	bool empty() const {
		return c_.empty();
	}

	/// Clear all elements, deleting them.
	void clear() {
		c_.clear();
	}

	/// Push an item on to the stack, deleting the
	/// bottom item on overflow.
	void push(value_type const & v) {
		c_.push_front(v);
		if (c_.size() > limit_) {
			c_.pop_back();
		}
	}

	/// Direct read access to intermediate elements.
	T const & operator[](size_type pos) const {
		return c_[pos];
	}

	/// Read access to used size.
	size_type size() const {
		return c_.size();
	}

	const_iterator begin() const {
		return c_.begin();
	}

	const_iterator end() const {
		return c_.end();
	}

private:
	/// Internal contents.
	container_type c_;
	/// The maximum number elements stored.
	size_type limit_;
};

// Make pointer type an error.
template <typename T>
class limited_stack<T*>;


} // namespace lyx

#endif // LIMITED_STACK_H
