// -*- C++ -*-

#ifndef INSET_LIST_H
#define INSET_LIST_H

#include "support/types.h"

class Inset;
class BufferView;


///
class InsetList {
public:
	///
	struct InsetTable {
		///
		lyx::pos_type pos;
		///
		Inset * inset;
		///
		InsetTable(lyx::pos_type p, Inset * i) : pos(p), inset(i) {}
	};
	///
	typedef std::vector<InsetTable> List;
	///
	typedef List::iterator iterator;
	///
	typedef List::const_iterator const_iterator;
	
	///
	~InsetList();
	///
	iterator begin();
	///
	iterator end();
	///
	const_iterator begin() const;
	///
	const_iterator end() const;
	///
	iterator insetIterator(lyx::pos_type pos);
	///
	void insert(Inset * inset, lyx::pos_type pos);
	///
	void erase(lyx::pos_type pos);
	///
	Inset * release(lyx::pos_type);
	///
	Inset * get(lyx::pos_type pos) const;
	///
	void increasePosAfterPos(lyx::pos_type pos);
	///
	void decreasePosAfterPos(lyx::pos_type pos);
	///
	void deleteInsetsLyXText(BufferView * bv);
	///
	void resizeInsetsLyXText(BufferView * bv);
private:
	///
	List list;
};

#endif
