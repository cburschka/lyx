// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1996-2000 the LyX Team.
 * 
 * ====================================================== */

#ifndef INSET_INDEX_H
#define INSET_INDEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"

class Buffer;
struct LaTeXFeatures;

// Created by Lgb 970227


/** Used to insert index labels  
  */
class InsetIndex: public InsetCommand {
public:
	///
	InsetIndex() : InsetCommand("index") {}
	///
	explicit
	InsetIndex(string const & key);
	///
	~InsetIndex();
	///
	Inset * Clone() const { return new InsetIndex(contents);}
	///
	void Edit(BufferView *, int, int, unsigned int);
	///
	EDITABLE Editable() const
	{
		return IS_EDITABLE;
	}
	///
	string getScreenLabel() const;
};


class InsetPrintIndex : public InsetCommand {
public:
	///
	InsetPrintIndex();
	///
	InsetPrintIndex(Buffer *);
	///
	~InsetPrintIndex();
	/// Updates needed features for this inset.
	void Validate(LaTeXFeatures & features) const;
	///
	void Edit(BufferView *, int, int, unsigned int) {}
	///
	EDITABLE Editable() const{
		return IS_EDITABLE;
	}
	/// WHY is clone missing? (Lgb)
	///
	bool display() const { return true; }
	///
	Inset::Code LyxCode() const;
	///
	string getScreenLabel() const;
private:
	///
	Buffer * owner;
};

// this was shifted here rather than a separate
// file because its little and only need by
// insetindex.C and lyx_gui_misc.C ARRae 981020
struct FD_index_form {
	FL_FORM * index_form;
	FL_OBJECT * key;
};

extern FD_index_form * index_form;
#endif
