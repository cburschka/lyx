// -*- C++ -*-

#ifndef ITERATORS_H
#define ITERATORS_H

#include <stack>

#include "paragraph.h"

class ParPosition {
public:
	ParPosition(Paragraph * p)
		: par(p), it(p->inset_iterator_begin()), index(-1) {}
	///
	Paragraph * par;
	///
	Paragraph::inset_iterator it;
	///
	int index;
};


inline
bool operator==(ParPosition const & pos1, ParPosition const & pos2) {
	return pos1.par == pos2.par &&
		pos1.it == pos2.it &&
		pos1.index == pos2.index;
}

inline
bool operator!=(ParPosition const & pos1, ParPosition const & pos2) {
	return !(pos1 == pos2);
}


class ParIterator {
public:
	///
	typedef std::stack<ParPosition> PosHolder;
	///
	ParIterator() {}
	///
	ParIterator(Paragraph * par) {
		positions.push(ParPosition(par));
	}
	///
	ParIterator & operator++();
	///
	Paragraph * operator*() {
		return positions.top().par;
	}
	///
	PosHolder::size_type size() const 
		{ return positions.size(); }
	///
	friend
	bool operator==(ParIterator const & iter1, ParIterator const & iter2);
private:
	///
	PosHolder positions;
};


///
inline
bool operator==(ParIterator const & iter1, ParIterator const & iter2) {
	return iter1.positions == iter2.positions;
}


///
inline
bool operator!=(ParIterator const & iter1, ParIterator const & iter2) {
	return !(iter1 == iter2);
}

#endif
