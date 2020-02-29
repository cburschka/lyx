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

#include "insets/Inset.h"

#include "support/debug.h"

#include <algorithm>

using namespace std;

namespace lyx {


namespace {

typedef InsetList::Element Table;

struct ElementPosLess
{
	bool operator()(Table const & t1, Table const & t2) const
	{
		return t1.pos < t2.pos;
	}
};

} // namespace


InsetList::InsetList(InsetList const & il) : list_(il.list_)
{
	for (auto & i : list_)
		i.inset = i.inset->clone();
}


InsetList::InsetList(InsetList const & il, pos_type beg, pos_type end)
{
	for (auto const & ci : il) {
		if (ci.pos < beg)
			continue;
		if (ci.pos >= end)
			break;
		// Add a new entry in the insetlist_.
		insert(ci.inset->clone(), ci.pos - beg);
	}
}


InsetList::~InsetList()
{
	for (auto & i : list_)
		delete i.inset;
}


void InsetList::setBuffer(Buffer & b)
{
	for (auto & i : list_)
		i.inset->setBuffer(b);
}


void InsetList::resetBuffer()
{
	for (auto & i : list_)
		i.inset->resetBuffer();
}


InsetList::iterator InsetList::insetIterator(pos_type pos)
{
	Element search_elem(pos, nullptr);
	return lower_bound(list_.begin(), list_.end(), search_elem,
			   ElementPosLess());
}


InsetList::const_iterator InsetList::insetIterator(pos_type pos) const
{
	Element search_elem(pos, nullptr);
	return lower_bound(list_.begin(), list_.end(), search_elem,
			   ElementPosLess());
}


void InsetList::insert(Inset * inset, pos_type pos)
{
	List::iterator end = list_.end();
	List::iterator it = insetIterator(pos);
	if (it != end && it->pos == pos) {
		LYXERR0("ERROR (InsetList::insert): "
		       << "There is an inset in position: " << pos);
	} else {
		list_.insert(it, Element(pos, inset));
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
		it->inset = nullptr;
		return tmp;
	}
	return nullptr;
}


Inset * InsetList::get(pos_type pos) const
{
	List::const_iterator end = list_.end();
	List::const_iterator it = insetIterator(pos);
	if (it != end && it->pos == pos)
		return it->inset;
	return nullptr;
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
