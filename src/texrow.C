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
	TexRow_Item *current, *iter = next;
	while (iter) {
		// Iterate through the list deleting as you go.
		// A bit easier to debug than recursive deletion.
		current = iter;
		iter = iter->next;
		delete current;
	}
	count = 0;
	next = 0;
	lastpar = 0;
	lastpos = -1;
}

// Defines paragraph and position for the beginning of this row
void TexRow::start(LyXParagraph *par, int pos) {
	lastpar = par;
	lastpos = pos;
}

// Insert node when line is completed
void TexRow::newline()
{
	TexRow_Item *tmp = new TexRow_Item;
	tmp->pos = lastpos;
	
	if (lastpar)
		tmp->id = lastpar->GetID();
	else
		tmp->id = -1;

	// Inserts at the beginning of the list
	tmp->next = next;
	next = tmp;
	count++;
	tmp->rownumber = count;
}


void TexRow::getIdFromRow(int row, int &id, int &pos)
{
	TexRow_Item *tmp = next;
	while (tmp && tmp->rownumber != row) {
		tmp = tmp->next;
	}
	if (tmp) {
		TexRow_Item *tmp2 = next;
		// Increase the pos of all rows with the
		// same id (and where the pos is larger)
		// to avoid putting errorinsets at the
		// same pos.
		while (tmp2) {
			if (tmp2 != tmp &&
			    tmp2->id == tmp->id &&
			    tmp2->pos >= tmp->pos)
				tmp2->pos++;
			tmp2 = tmp2->next;
		}
		id = tmp->id;
		pos = tmp->pos;
	} else {
		id = -1;
		pos = 0;
	}
}


TexRow & TexRow::operator+= (const TexRow &tr)
{
	// remember that the lists are stored in reverse 
	// so you've got to turn the second one around 
	// as you insert it in the first
	for (int counter = tr.count; counter > 0; --counter) {
		int i = 1;
		TexRow_Item *iter = tr.next;
		while (i < counter) {
			iter = iter->next;
			++i;
		}

		++count;
		TexRow_Item *tmp;
		tmp = new TexRow_Item;
		tmp->id = iter->id;
		tmp->pos = iter->pos;
		tmp->next = next;
		tmp->rownumber = count;
		next = tmp;
	}
	// should I be doing this or not?
	//lastpar = tr.lastpar;
	//lastpos = tr.lastpos;
	return *this;
}	
