// -*- C++ -*-

#ifndef UPDATE_INSET_H
#define UPDATE_INSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include <queue>

class Inset;
class BufferView;

///
class UpdateInset {
public:
	///
	void push(Inset * inset);

	///
	void update(BufferView *);
private:
	///
	typedef std::queue<Inset*> InsetQueue;
	///
	InsetQueue insetqueue;
};

#endif // UPDATE_INSET_H
