/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif

#include "texrow.h"
#include "lyxparagraph.h"
#include "debug.h"

using std::find_if;
using std::endl;

// Delete linked list
void TexRow::reset()
{
	rowlist.clear();
	count = 0;
	lastpar = 0;
	lastpos = -1;
}


// Defines paragraph and position for the beginning of this row
void TexRow::start(LyXParagraph * par, int pos)
{
	lastpar = par;
	lastpos = pos;
}


// Insert node when line is completed
void TexRow::newline()
{
	RowItem tmp;
	tmp.pos(lastpos);
	if (lastpar)
		tmp.id(lastpar->id());
	else
		tmp.id(-1);
	tmp.rownumber(++count);
	rowlist.push_back(tmp);
}


class same_rownumber {
public:
	same_rownumber(TexRow::RowList::value_type const & v):vt(v){}
	bool operator()(TexRow::RowList::value_type const & vt1) const {
		return vt.rownumber() == vt1.rownumber();
	}
private:
	TexRow::RowList::value_type const & vt;
};



bool TexRow::getIdFromRow(int row, int & id, int & pos) const
{
	RowList::value_type vt;
	vt.rownumber(row);
	RowList::const_iterator cit =
		find_if(rowlist.begin(), rowlist.end(), same_rownumber(vt));
	
	if (cit != rowlist.end()) {
#if 0
		RowList::iterator kit = rowlist.begin();
		RowList::iterator end = rowlist.end();
		// Increase the pos of all rows with the
		// same id (and where the pos is larger)
		// to avoid putting errorinsets at the
		// same pos.
		for(; kit != end; ++kit) {
			if (&(*kit) != &(*cit)
			    && (*kit).id() == (*cit).id()
			    && (*kit).pos() >= (*cit).pos())
				(*kit).pos((*kit).pos() + 1);
		}
#endif
		id = (*cit).id();
		pos = (*cit).pos();
		return true;
	}
	id = -1;
	pos = 0;
	return false;
}


// should perhaps have a better name...
// Increase the pos of all rows with the
// same id (and where the pos is larger)
// to avoid putting errorinsets at the
// same pos.
void TexRow::increasePos(int id, int pos) const
{
	RowList::iterator kit = rowlist.begin();
	RowList::iterator end = rowlist.end();
	for(; kit != end; ++kit) {
		if (id == (*kit).id()
		    && pos < (*kit).pos()) {
			(*kit).pos((*kit).pos() + 1);
			lyxerr << "TeXRow::increasePos: ideally this "
				"should never happen..." << endl;
		}
		// When verified to work this clause should be deleted.
		if (id == (*kit).id()
		    && pos == (*kit).pos()) {
			lyxerr << "TexRow::increasePos: this should happen "
				"maximum one time for each run of "
				"increasePos!" << endl;
		}
	}
}


TexRow & TexRow::operator+= (TexRow const & tr)
{
	rowlist.insert(rowlist.end(), tr.rowlist.begin(), tr.rowlist.end());
	return *this;
}	
