/**
 * \file InsetList.C
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

#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "debug.h"

#include "insets/insetbranch.h"

using lyx::pos_type;

using std::endl;
using std::lower_bound;


namespace {

struct MatchIt {
	/// used by lower_bound
	inline
	int operator()(InsetList::InsetTable const & a,
		       InsetList::InsetTable const & b) const
	{
		return a.pos < b.pos;
	}
};

} // namespace anon


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
	return list.begin();
}


InsetList::iterator InsetList::end()
{
	return list.end();
}


InsetList::const_iterator InsetList::begin() const
{
	return list.begin();
}


InsetList::const_iterator InsetList::end() const
{
	return list.end();
}


InsetList::iterator InsetList::insetIterator(pos_type pos)
{
	InsetTable search_elem(pos, 0);
	return lower_bound(list.begin(), list.end(), search_elem, MatchIt());
}


InsetList::const_iterator InsetList::insetIterator(pos_type pos) const
{
	InsetTable search_elem(pos, 0);
	return lower_bound(list.begin(), list.end(), search_elem, MatchIt());
}


void InsetList::insert(InsetOld * inset, lyx::pos_type pos)
{
	List::iterator end = list.end();
	List::iterator it = insetIterator(pos);
	if (it != end && it->pos == pos) {
		lyxerr << "ERROR (InsetList::insert): "
		       << "There is an inset in position: " << pos << endl;
	} else {
		list.insert(it, InsetTable(pos, inset));
	}
}


void InsetList::erase(pos_type pos)
{
	List::iterator end = list.end();
	List::iterator it = insetIterator(pos);
	if (it != end && it->pos == pos) {
		delete it->inset;
		list.erase(it);
	}
}


InsetOld * InsetList::release(pos_type pos)
{
	List::iterator end = list.end();
	List::iterator it = insetIterator(pos);
	if (it != end && it->pos == pos) {
		InsetOld * tmp = it->inset;
		it->inset = 0;
		return tmp;
	}
	return 0;
}


InsetOld * InsetList::get(pos_type pos) const
{
	List::const_iterator end = list.end();
	List::const_iterator it = insetIterator(pos);
	if (it != end && it->pos == pos)
		return it->inset;
	return 0;
}


void InsetList::increasePosAfterPos(pos_type pos)
{
	List::iterator end = list.end();
	List::iterator it = insetIterator(pos);
	for (; it != end; ++it) {
		++it->pos;
	}
}


void InsetList::decreasePosAfterPos(pos_type pos)
{
	List::iterator end = list.end();
	List::iterator it = insetIterator(pos);
	for (; it != end; ++it) {
		--it->pos;
	}
}


void InsetList::insetsOpenCloseBranch(Buffer const & buf)
{
	List::iterator it = list.begin();
	List::iterator end = list.end();
	for (; it != end; ++it) {
		if (it->inset && it->inset->lyxCode() == InsetOld::BRANCH_CODE) {
			InsetBranch * inset = static_cast<InsetBranch *>(it->inset);
			if (buf.params().branchlist().selected(inset->params().branch)) {
				inset->open();
			} else {
				inset->close();
			}
		}
	}
}
