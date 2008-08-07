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

#include "insets/InsetCode.h"

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
	/// Partial copy constructor.
	/// Copy the InsetList contents from \p beg to \p end (without end).
	InsetList(InsetList const &, pos_type beg, pos_type end);
	///
	void setBuffer(Buffer &);

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

	/// search for next occurence of an \c Inset type.
	/// \return the position of the found inset.
	/// \retval -1 if no \c Inset is found.
	pos_type find(
		InsetCode code, ///< Code of inset to find.
		pos_type startpos = 0 ///< start position for the search.
		) const;

	/// count occurences of of an \c Inset type.
	/// \return the number of found inset(s).
	int count(
		InsetCode code, ///< Code of inset type to count.
		pos_type startpos = 0 ///< start position for the counting.
		) const;

private:
	///
	List list_;
};


} // namespace lyx

#endif
