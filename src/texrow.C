/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *          Copyright (C) 1995-1999 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "texrow.h"
#include "lyxparagraph.h"
#include "debug.h"


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
	tmp.pos = lastpos;
	if (lastpar)
		tmp.id = lastpar->id();
	else
		tmp.id = -1;
	tmp.rownumber = ++count;
	rowlist.push_back(tmp);
}


void TexRow::getIdFromRow(int row, int & id, int & pos)
{
	RowList::const_iterator cit = rowlist.begin();
	for (; cit != rowlist.end(); ++cit) {
		if ((*cit).rownumber == row) break;
	}
	if (cit != rowlist.end()) {
		RowList::iterator kit = rowlist.begin();
		// Increase the pos of all rows with the
		// same id (and where the pos is larger)
		// to avoid putting errorinsets at the
		// same pos.
		for(; kit != rowlist.end(); ++kit) {
			if (&(*kit) != &(*cit)
			    && (*kit).id == (*cit).id
			    && (*kit).pos >= (*cit).pos)
				(*kit).pos++;
		}
	} else {
		id = -1;
		pos = 0;
	}
}


TexRow & TexRow::operator+= (TexRow const & tr)
{
	rowlist.insert(rowlist.end(), tr.rowlist.begin(), tr.rowlist.end());
	return *this;
}	
