// -*- C++ -*-

#ifndef BLOCK_H
#define BLOCK_H

#include "LAssert.h"

template <class T, size_t s>
class block {
public:
	typedef T value_type;
	typedef size_t size_type;
	typedef T * pointer;
	typedef T const * const_pointer;
	typedef T & reference;
	typedef T const & const_reference;
	typedef T * iterator;
	typedef T const * const_iterator;
	size_type size() const { return s; }
#warning I disabled this to be able to compile... (JMarc)
  // I think that, sor the same reason that string->char* is not
  // automatic, we should have a c_array() method to do that. However,
  // Lars, it is your choice...  
  //	operator T* () { return arr; }
	reference at(int i) {
		Assert(i >= 0 && i < s);
		return arr[i];
	}
	const_reference at(int i) const {
		Assert(i >= 0 && i < s);
		return arr[i];
	}
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
	T arr[s];
};

#endif // BLOCK_H_
