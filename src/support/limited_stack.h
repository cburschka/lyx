// -*- C++ -*-
/**
 * \file limited_stack.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef LIMITED_STACK_H
#define LIMITED_STACK_H

#include <list>

#include <boost/shared_ptr.hpp>
 
/**
 * limited_stack - a stack of limited size
 *
 * Like a normal stack, but only accepts pointer types,
 * and bottom elements are deleted on overflow
 */
template <typename T>
class limited_stack {
public:
	typedef std::list<T> container_type;
	typedef typename container_type::value_type value_type;
	typedef typename container_type::size_type size_type;
 
	/// limit is the maximum size of the stack
	limited_stack(size_type limit = 10) {
		limit_ = limit;
	}

	/// return the top element
	value_type top() {
		return c_.front();
	}

	/// pop and throw away the top element
	void pop() {
		c_.pop_front();
	}
 
	/// return true if the stack is empty
	bool empty() const {
		return c_.size() == 0;
	}

	/// clear all elements, deleting them
	void clear() {
		while (!c_.empty()) {
			c_.pop_back();
		}
	}
 
	/// push an item on to the stack, deleting the
	/// bottom item on overflow.
	void push(value_type const & v) {
		c_.push_front(v);
		if (c_.size() > limit_) {
			c_.pop_back(); 
		}
	}
 
private:
	/// internal contents
	container_type c_;
	/// the maximum number elements stored
	size_type limit_;
};

// make pointer type an error.
template <typename T>
class limited_stack<T*>;
 
#endif // LIMITED_STACK_H
