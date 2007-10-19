// -*- C++ -*-
/**
 * \file InsetList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_LIST_H
#define INSET_LIST_H

#include "support/types.h"

#include <vector>


namespace lyx {

class Inset;
class Buffer;

///
class InsetList {
public:
	///
	InsetList() {}
	///
	InsetList(InsetList const &);

	///
	class InsetTable {
	public:
		///
		InsetTable(pos_type p, Inset * i) : pos(p), inset(i) {}
		///
		pos_type pos;
		///
		Inset * inset;
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
	iterator begin() { return list_.begin(); }
	///
	iterator end() { return list_.end(); }
	///
	const_iterator begin() const { return list_.begin(); }
	///
	const_iterator end() const { return list_.end(); }
	///
	bool empty() const { return list_.empty(); }
	///
	iterator insetIterator(pos_type pos);
	///
	const_iterator insetIterator(pos_type pos) const;
	///
	void insert(Inset * inset, pos_type pos);
	///
	void erase(pos_type pos);
	///
	Inset * release(pos_type);
	///
	Inset * get(pos_type pos) const;
	///
	void increasePosAfterPos(pos_type pos);
	///
	void decreasePosAfterPos(pos_type pos);

private:
	///
	List list_;
};


} // namespace lyx

#endif
