#include <config.h>

#include "InsetList.h"
#include "debug.h"

#include "insets/inset.h"

#include <algorithm>

using lyx::pos_type;

using std::lower_bound;
using std::upper_bound;
using std::endl;

namespace {

struct MatchIt {
	/// used by lower_bound and upper_bound
	inline
	int operator()(InsetList::InsetTable const & a,
		       InsetList::InsetTable const & b) const {
		return a.pos < b.pos;
	}
};

}


InsetList::iterator::iterator(InsetList::List::iterator const & iter)
	: it(iter)
{}


InsetList::iterator & InsetList::iterator::operator++()
{
	++it;
	return *this;
}


InsetList::iterator InsetList::iterator::operator++(int)
{
	iterator tmp = *this;
	++*this;
	return tmp;
}


pos_type InsetList::iterator::getPos() const
{
	return it->pos;
}


Inset * InsetList::iterator::getInset() const
{
	return it->inset;
}


void InsetList::iterator::setInset(Inset * inset)
{
	it->inset = inset;
}


InsetList::~InsetList() 
{
	// If we begin storing a shared_ptr in the List
	// this code can be removed. (Lgb)
	List::iterator it = list.begin();
	List::iterator end = list.end();
	for (; it != end; ++it) {
		delete it->inset;
	}
}


InsetList::iterator InsetList::begin()
{
	return iterator(list.begin());
}


InsetList::iterator InsetList::end()
{
	return iterator(list.end());
}


InsetList::iterator InsetList::begin() const
{
	return iterator(const_cast<InsetList*>(this)->list.begin());
}


InsetList::iterator InsetList::end() const
{
	return iterator(const_cast<InsetList*>(this)->list.end());
}


InsetList::iterator
InsetList::insetIterator(pos_type pos)
{
	InsetTable search_elem(pos, 0);
	List::iterator it = lower_bound(list.begin(),
					list.end(),
					search_elem, MatchIt());
	return iterator(it);
}


void InsetList::insert(Inset * inset, lyx::pos_type pos)
{
	InsetTable search_elem(pos, 0);
	List::iterator it = lower_bound(list.begin(),
					list.end(),
					search_elem, MatchIt());
	if (it != list.end() && it->pos == pos) {
		lyxerr << "ERROR (InsetList::insert): "
		       << "There is an inset in position: " << pos << endl;
	} else {
		list.insert(it, InsetTable(pos, inset));
	}
}


void InsetList::erase(pos_type pos)
{
	InsetTable search_elem(pos, 0);
	List::iterator it =
		lower_bound(list.begin(),
			    list.end(),
			    search_elem, MatchIt());
	if (it != list.end() && it->pos == pos) {
		delete it->inset;
		list.erase(it);
	}
}


Inset * InsetList::release(pos_type pos)
{
	InsetTable search_elem(pos, 0);
	List::iterator it =
		lower_bound(list.begin(),
			    list.end(),
			    search_elem, MatchIt());
	if (it != list.end() && it->pos == pos) {
		Inset * tmp = it->inset;
		it->inset = 0;
		return tmp;
	}
	return 0;
}

	
Inset * InsetList::get(pos_type pos) const
{
	InsetTable search_elem(pos, 0);
	List::iterator it =
		lower_bound(const_cast<InsetList*>(this)->list.begin(),
			    const_cast<InsetList*>(this)->list.end(),
			    search_elem, MatchIt());
	if (it != const_cast<InsetList*>(this)->list.end() && it->pos == pos)
		return it->inset;
	return 0;
}


void InsetList::increasePosAfterPos(pos_type pos)
{
	InsetTable search_elem(pos, 0);
	List::iterator it = lower_bound(list.begin(),
					list.end(),
					search_elem, MatchIt());
	List::iterator end = list.end();
	for (; it != end; ++it) {
		++it->pos;
	}
}


void InsetList::decreasePosAfterPos(pos_type pos)
{
	InsetTable search_elem(pos, 0);
	List::iterator end = list.end();
	List::iterator it = upper_bound(list.begin(),
					end,
					search_elem, MatchIt());
	for (; it != end; ++it) {
		--it->pos;
	}
}


void InsetList::deleteInsetsLyXText(BufferView * bv)
{
	List::iterator it = list.begin();
	List::iterator end = list.end();
	for (; it != end; ++it) {
		if (it->inset) {
			if (it->inset->isTextInset()) {
				static_cast<UpdatableInset*>
					(it->inset)->deleteLyXText(bv, true);
			}
		}
	}
}


void InsetList::resizeInsetsLyXText(BufferView * bv)
{
	List::iterator it = list.begin();
	List::iterator end = list.end();
	for (; it != end; ++it) {
		if (it->inset) {
			if (it->inset->isTextInset()) {
				static_cast<UpdatableInset*>
					(it->inset)->resizeLyXText(bv, true);
			}
		}
	}
}



bool operator==(InsetList::iterator const & i1,
		InsetList::iterator const & i2)
{
	return i1.it == i2.it;
	
}
	

bool operator!=(InsetList::iterator const & i1,
		InsetList::iterator const & i2)
{
	return !(i1 == i2);
}
