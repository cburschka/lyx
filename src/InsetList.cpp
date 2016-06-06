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
#include <algorithm>

#include "InsetList.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BranchList.h"

#include "insets/InsetBranch.h"

#include "support/debug.h"

using namespace std;

namespace lyx {


namespace {

typedef InsetList::InsetTable Table;

struct InsetTablePosLess
{
	bool operator()(Table const & t1, Table const & t2) const
	{
		return t1.pos < t2.pos;
	}
};

} // namespace anon


InsetList::InsetList(InsetList const & il) : list_(il.list_)
{
	List::iterator it = list_.begin();
	List::iterator end = list_.end();
	for (; it != end; ++it)
		it->inset = it->inset->clone();
}


InsetList::InsetList(InsetList const & il, pos_type beg, pos_type end)
{
	InsetList::const_iterator cit = il.begin();
	InsetList::const_iterator cend = il.end();
	for (; cit != cend; ++cit) {
		if (cit->pos < beg)
			continue;
		if (cit->pos >= end)
			break;
		// Add a new entry in the insetlist_.
		insert(cit->inset->clone(), cit->pos - beg);
	}
}


InsetList::~InsetList()
{
	List::iterator it = list_.begin();
	List::iterator end = list_.end();
	for (; it != end; ++it)
		delete it->inset;
}


void InsetList::setBuffer(Buffer & b)
{
	List::iterator it = list_.begin();
	List::iterator end = list_.end();
	for (; it != end; ++it)
		it->inset->setBuffer(b);
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
		LYXERR0("ERROR (InsetList::insert): "
		       << "There is an inset in position: " << pos);
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
	for (; it != end; ++it)
		++it->pos;
}


void InsetList::decreasePosAfterPos(pos_type pos)
{
	List::iterator end = list_.end();
	List::iterator it = insetIterator(pos);
	for (; it != end; ++it)
		--it->pos;
}


pos_type InsetList::find(InsetCode code, pos_type startpos) const
{
	List::const_iterator it = insetIterator(startpos);
	List::const_iterator end = list_.end();
	for (; it != end ; ++it) {
		if (it->inset->lyxCode() == code)
			return it->pos;
	}
	return -1;
}


int InsetList::count(InsetCode code, pos_type startpos) const
{
	int num = 0;
	List::const_iterator it = insetIterator(startpos);
	List::const_iterator end = list_.end();
	for (; it != end ; ++it) {
		if (it->inset->lyxCode() == code)
			++num;
	}
	return num;
}

} // namespace lyx
