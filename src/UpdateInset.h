// -*- C++ -*-

#ifndef UPDATE_INSET_H
#define UPDATE_INSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include <queue>
#include "support/LAssert.h"

using std::queue;

class Inset;
class BufferView;

///
class UpdateInset {
public:
	///
	void push(Inset * inset) {
		Assert(inset);
		insetqueue.push(inset);
	}
	///
	void update(BufferView *);
private:
	///
	typedef queue<Inset*> InsetQueue;
	///
	InsetQueue insetqueue;
};

#endif // UPDATE_INSET_H
