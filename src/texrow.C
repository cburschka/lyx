/**
 * \file texrow.C
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Matthias Ettrich
 */

#include <config.h>

#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif

#include "texrow.h"
#include "paragraph.h"
#include "debug.h"

using std::find_if;
using std::for_each;
using std::endl;

namespace {
 
/// function object returning true when row number is found
class same_rownumber {
public:
	same_rownumber(int row) : row_(row) {}
	bool operator()(TexRow::RowList::value_type const & vt) const {
		return vt.rownumber() == row_;
	}
 
private:
	int row_;
};


/// increment the pos value of the argument if the par id
/// is the same, and the pos parameter is larger
class increase_pos {
public:
	increase_pos(int id, int pos)
		: id_(id), pos_(pos) {}

	void operator()(TexRow::RowList::value_type & vt) const {
		if (vt.id() != id_ || vt.pos() >= pos_)
			return;
		vt.pos(vt.pos() + 1);
 
		lyxerr[Debug::INFO]
			<< "TeXRow::increasePos: ideally this "
			"should never happen..." << endl;

		// FIXME: When verified to work this clause should be deleted.
		if (id_ == vt.id() && pos_ == vt.pos()) {
			lyxerr[Debug::INFO]
				<< "TexRow::increasePos: this should happen "
				"maximum one time for each run of "
				"increasePos!" << endl;
		}
	}

private:
	int id_;
	int pos_;
};
 
} // namespace anon 

 
void TexRow::reset()
{
	rowlist.clear();
	count = 0;
	lastpar = 0;
	lastpos = -1;
}


void TexRow::start(Paragraph * par, int pos)
{
	lastpar = par;
	lastpos = pos;
}


void TexRow::newline()
{
	int const id = lastpar ? lastpar->id() : -1;
	RowList::value_type tmp(id, lastpos, ++count);
	rowlist.push_back(tmp);
}


bool TexRow::getIdFromRow(int row, int & id, int & pos) const
{
	RowList::const_iterator cit =
		find_if(rowlist.begin(), rowlist.end(), 
			same_rownumber(row));
 
	if (cit != rowlist.end()) {
		id = cit->id();
		pos = cit->pos();
		return true;
	}
	id = -1;
	pos = 0;
	return false;
}


void TexRow::increasePos(int id, int pos)
{
	for_each(rowlist.begin(), rowlist.end(), increase_pos(id, pos));
}


TexRow & TexRow::operator+=(TexRow const & tr)
{
	rowlist.insert(rowlist.end(), tr.rowlist.begin(), tr.rowlist.end());
	return *this;
}
