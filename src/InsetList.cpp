/**
 * \file InsetList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetList.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BranchList.h"
#include "debug.h"

#include "insets/InsetBranch.h"

using std::endl;
using std::lower_bound;


namespace lyx {


namespace {

typedef InsetList::InsetTable Table;

class InsetTablePosLess : public std::binary_function<Table, Table, bool> {
public:
	bool operator()(Table const & t1, Table const & t2) const
	{
		return t1.pos < t2.pos;
	}
};

} // namespace anon



InsetList::~InsetList()
{
	// If we begin storing a shared_ptr in the List
	// this code can be removed. (Lgb)
	List::iterator it = list_.begin();
	List::iterator end = list_.end();
	for (; it != end; ++it) {
		delete it->inset;
	}
}


InsetList::iterator InsetList::insetIterator(pos_type pos)
{
	InsetTable search_elem(pos, 0);
	return lower_bound(list_.begin(), list_.end(), search_elem,
			   InsetTablePosLess());
}


InsetList::const_iterator InsetList::insetIterator(pos_type pos) const
{
	InsetTable search_elem(pos, 0);
	return lower_bound(list_.begin(), list_.end(), search_elem,
			   InsetTablePosLess());
}


void InsetList::insert(Inset * inset, pos_type pos)
{
	List::iterator end = list_.end();
	List::iterator it = insetIterator(pos);
	if (it != end && it->pos == pos) {
		lyxerr << "ERROR (InsetList::insert): "
		       << "There is an inset in position: " << pos << endl;
	} else {
		list_.insert(it, InsetTable(pos, inset));
	}
}


void InsetList::erase(pos_type pos)
{
	List::iterator end = list_.end();
	List::iterator it = insetIterator(pos);
	if (it != end && it->pos == pos) {
		delete it->inset;
		list_.erase(it);
	}
}


Inset * InsetList::release(pos_type pos)
{
	List::iterator end = list_.end();
	List::iterator it = insetIterator(pos);
	if (it != end && it->pos == pos) {
		Inset * tmp = it->inset;
		it->inset = 0;
		return tmp;
	}
	return 0;
}


Inset * InsetList::get(pos_type pos) const
{
	List::const_iterator end = list_.end();
	List::const_iterator it = insetIterator(pos);
	if (it != end && it->pos == pos)
		return it->inset;
	return 0;
}


void InsetList::increasePosAfterPos(pos_type pos)
{
	List::iterator end = list_.end();
	List::iterator it = insetIterator(pos);
	for (; it != end; ++it) {
		++it->pos;
	}
}


void InsetList::decreasePosAfterPos(pos_type pos)
{
	List::iterator end = list_.end();
	List::iterator it = insetIterator(pos);
	for (; it != end; ++it) {
		--it->pos;
	}
}


void InsetList::clone()
{
	List::iterator it = list_.begin();
	List::iterator end = list_.end();
	for (; it != end; ++it)
		it->inset = it->inset->clone();
}

} // namespace lyx
