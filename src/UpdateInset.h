// -*- C++ -*-

#ifndef UPDATE_INSET_H
#define UPDATE_INSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include <queue>
#include "support/LAssert.h"

class Inset;
class BufferView;

///
class UpdateInset {
public:
	///
	void push(Inset * inset) {
		lyx::Assert(inset);
		insetqueue.push(inset);
	}
	///
	void update(BufferView *);
private:
	///
	typedef std::queue<Inset*> InsetQueue;
	///
	InsetQueue insetqueue;
};

#endif // UPDATE_INSET_H
