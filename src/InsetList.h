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
	class iterator {
	public:
		///
		iterator() {}
		//
		iterator(List::iterator const & iter);
		///
		iterator & operator++();
		///
		iterator operator++(int);
		///
		lyx::pos_type getPos() const;
		///
		Inset * getInset() const;
		///
		void setInset(Inset * inset);
		///
		friend bool operator==(iterator const &, iterator const &);
	private:
		///
		List::iterator it;
	};
	///
	~InsetList();
	///
	iterator begin();
	///
	iterator end();
	///
	iterator begin() const;
	///
	iterator end() const;
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

///
bool operator==(InsetList::iterator const & i1,
		InsetList::iterator const & i2);
///
bool operator!=(InsetList::iterator const & i1,
		InsetList::iterator const & i2);

#endif
