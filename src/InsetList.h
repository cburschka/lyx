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

class InsetBase;
class Buffer;


///
class InsetList {
public:
	///
	struct InsetTable {
		///
		lyx::pos_type pos;
		///
		InsetBase * inset;
		///
		InsetTable(lyx::pos_type p, InsetBase * i) : pos(p), inset(i) {}
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
	iterator insetIterator(lyx::pos_type pos);
	///
	const_iterator insetIterator(lyx::pos_type pos) const;
	///
	void insert(InsetBase * inset, lyx::pos_type pos);
	///
	void erase(lyx::pos_type pos);
	///
	InsetBase * release(lyx::pos_type);
	///
	InsetBase * get(lyx::pos_type pos) const;
	///
	void increasePosAfterPos(lyx::pos_type pos);
	///
	void decreasePosAfterPos(lyx::pos_type pos);

private:
	///
	List list_;
};

#endif
