// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich, 1995, 1996 LyX Team
 *
 *======================================================*/
#ifndef _UNDO_H
#define _UNDO_H

#ifdef __GNUG__
#pragma interface
#endif

#include "definitions.h"
#include "lyxparagraph.h"


///
class Undo {
public:
	/// The undo kinds
	enum undo_kind {
		///
		INSERT,
		///
		DELETE,
		///
		EDIT,
		///
		FINISH
	};
	///
	undo_kind kind;
	///
	int number_of_before_par;
	///
	int number_of_behind_par;
	///
	int number_of_cursor_par;
	///
	int cursor_pos; // valid if >= 0
	///
	LyXParagraph* par;
	///
	Undo(undo_kind kind_arg,
	     int number_before_arg, int number_behind_arg,
	     int cursor_par_arg, int cursor_pos_arg,
	     LyXParagraph* par_arg)
	{
		kind = kind_arg;
		number_of_before_par = number_before_arg;
		number_of_behind_par = number_behind_arg;
		number_of_cursor_par = cursor_par_arg;
		cursor_pos = cursor_pos_arg;
		par = par_arg;
	}
	///
	~Undo(){
		LyXParagraph* tmppar;
		while (par) {
			tmppar = par;
			par = par->next;
			delete tmppar;
		}
	}
};


/// A limited Stack for the undo informations. Matthias 290496
class UndoStack{
private:
	///
	struct StackAtom{
		///
		StackAtom* previous;
		///
		Undo* undo;
	};
	///
	StackAtom* current;
	///
	StackAtom *tmp;
	///
	int size;
	///
	int limit;
public:
	///
	UndoStack(){
		current = NULL;
		// size must be initialised (thornley)
		size = 0;
		limit = 100; // the maximum number of undo steps stored. 0 means NO LIMIT. 
		// Limit can be changed with UndoStack::SetStackLimit(int) 
	}
	///
	Undo *Pop(){
		Undo* result = NULL;
		if (current){
			result = current->undo;
			tmp = current;
			current = current->previous;
			delete tmp;
			size--;
		}
		else {
			size = 0; // for safety...
		}
		return result;
	}
	///
	Undo* Top(){
		if (current)
			return current->undo;
		else
			return NULL;
	}
	///
	~UndoStack(){
		Clear();
	}
	///
	void Clear(){
		Undo* tmp_undo = Pop();
		while (tmp_undo){
			delete tmp_undo;
			tmp_undo = Pop();
		}
	}
	///
	void SetStackLimit(int limit_arg) {
		limit = limit_arg;
	}
    
	///
	void Push(Undo* undo_arg){
		int i;
		StackAtom* tmp2;
		if (undo_arg){
			tmp = new StackAtom;
			tmp->undo = undo_arg;
			tmp->previous = current;
			current = tmp;
			size++;
			if (limit && size > limit){
				for (i=0; i<limit && tmp; i++)
					tmp = tmp->previous;
				while(tmp && tmp->previous){
					tmp2 = tmp->previous->previous;
					delete tmp->previous;
					size--;
					tmp->previous = tmp2;
				}
			}
		}
	}
};

#endif
