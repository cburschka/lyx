/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *	     Copyright (C) 1995 Matthias Ettrich
 *           Copyright (C) 1995-1998 The LyX Team.
 *
 *           This file is Copyright (C) 1996-1998
 *           Lars Gullik Bjønnes
 *
 * ======================================================
 */

#include <config.h>

#include "DepTable.h"
#include "lyxlib.h"
#include "filetools.h"

DepTable::DepTable()
{
	new_sum = 0;
	old_sum = 0;
	next = NULL;
}


DepTable::DepTable(LString const & f,
		   bool upd,
		   unsigned long one,
		   unsigned long two)
{
	// not quite sure if this is the correct place for MakeAbsPath
	file = MakeAbsPath(f);
	new_sum = 0; old_sum = 0;
	if (one != 0)
		new_sum = one;
	if (two != 0)
		old_sum = two;
	if (upd) {
		old_sum = new_sum;
		new_sum = lyxsum(file.c_str());
	}
	if (lyxerr.debugging()) {
		char tmp1[256];
		char tmp2[256];
		sprintf(tmp1, "%lu", new_sum);
		sprintf(tmp2, "%lu", old_sum);
		lyxerr.debug("New file inserted in deplog: " + 
			     file + " " +
			     tmp1 + " " + tmp2);
	}
	next = NULL;
}


void DepTable::insert(LString const & fi,
		      bool upd,
		      unsigned long one,
		      unsigned long two)
{
	// not quite sure if this is the correct place for MakeAbsPath
	LString f = MakeAbsPath(fi);
	if (f == file) return; // exist already in the log
	if (next)
		next->insert(f, upd, one, two);
	else
		next = new DepTable(f, upd, one, two);
}
		

void DepTable::update()
{
	if (!file.empty()) {
		old_sum = new_sum;
		new_sum = lyxsum(file.c_str());
		if (lyxerr.debugging()) {
			char tmp1[256];
			char tmp2[256];
			sprintf(tmp1, "%lu", new_sum);
			sprintf(tmp2, "%lu", old_sum);
			lyxerr.debug("update: " + file + " " +
				     tmp1 + " " + tmp2);
		}
	}
	if (next) next->update();
}


bool DepTable::sumchange()
{
	bool ret = false;
	
	if (!file.empty()) {
		if (old_sum != new_sum) ret = true;
	}
	if (!ret && next) ret = next->sumchange();

	return ret;
}


bool DepTable::haschanged(LString const & f)
{
	// not quite sure if this is the correct place for MakeAbsPath
	LString fil = MakeAbsPath(f);
	bool ret = false;

	if (!fil.empty() && !file.empty() && fil == file) {
		if (new_sum != old_sum && new_sum != 0)
			ret = true;
	}
	if (!ret && next) ret = next->haschanged(fil);
	return ret;
}


void DepTable::write(LString const&f)
{
	FilePtr fp(f, FilePtr::write);
	if (fp() && next) next->write(fp());
}


void DepTable::read(LString const &f)
{
	FilePtr fp(f, FilePtr::read);
	if (fp()) { // file opened
		char nome[256];
		unsigned long one = 0;
		unsigned long two = 0;
		// scan the file line by line
		// return true if the two numbers on the line is different
		int ret = 0;
		while (!feof(fp())) {
			ret = fscanf(fp(), "%s %lu %lu",
				     nome, &one, &two);
			if (ret !=3) continue;
			if (lyxerr.debugging()) {
				char tmp1[255];
				char tmp2[255];
				sprintf(tmp1, "%lu", one);
				sprintf(tmp2, "%lu", two);
				lyxerr.debug(LString("read dep: ") +
					     nome + " " + tmp1 +
					     " " + tmp2);
			}
			insert(LString(nome), false, one, two);
		}
	}
}


void DepTable::write(FILE * f)
{
	if (lyxerr.debugging()) {
		char tmp1[255];
		char tmp2[255];
		sprintf(tmp1, "%lu", new_sum);
		sprintf(tmp2, "%lu", old_sum);
		lyxerr.print("Write dep: " + file + " " +
			     tmp1 + " " + tmp2);
	}
	fprintf(f, "%s %lu %lu\n", file.c_str(),
		new_sum, old_sum);
	if (next)
		next->write(f);
}
