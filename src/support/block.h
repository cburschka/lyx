// -*- C++ -*-

#ifndef BLOCK_H
#define BLOCK_H

#include "LAssert.h"

template <class T, size_t s>
class block {
public:
	typedef T value_type;
	typedef size_t size_type;
	typedef value_type * pointer;
	typedef value_type const * const_pointer;
	typedef value_type & reference;
	typedef value_type const & const_reference;
	typedef value_type * iterator;
	typedef value_type const * const_iterator;
	size_type size() const { return s; }
	reference operator[](int i) { return arr[i]; }
	const_reference operator[](int i) const { return arr[i]; }
	void operator=(block const & b) {
		Assert(b.size() == size());
		for (size_t i = 0; i < size(); ++i) {
			arr[i] == b[i];
		}
	}
	bool operator==(block const & b) const {
		Assert(b.size() == size());
		for (size_t i = 0; i < size(); ++i) {
			if (arr[i] != b[i]) return false;
		}
		return true;
	}
	iterator begin() { return arr[0]; }
	iterator end() { return arr[s]; }
	const_iterator begin() const { return arr[0]; }
	const_iterator end() const { return arr[s]; }
private:
	value_type arr[s + 1];
};

#endif // BLOCK_H_
