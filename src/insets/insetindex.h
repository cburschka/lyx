// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1996-2001 the LyX Team.
 * 
 * ====================================================== */

#ifndef INSET_INDEX_H
#define INSET_INDEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"

struct LaTeXFeatures;

/** Used to insert index labels  
  */
class InsetIndex : public InsetCommand {
public:
	///
	InsetIndex(InsetCommandParams const &);
	///
	virtual Inset * clone(Buffer const &) const {
		return new InsetIndex(params());
	}
	///
	string const getScreenLabel() const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	void edit(BufferView *, int, int, unsigned int);
};


class InsetPrintIndex : public InsetCommand {
public:
	///
	InsetPrintIndex(InsetCommandParams const &);
	///
	virtual Inset * clone(Buffer const &) const {
		return new InsetPrintIndex(params());
	}
	/// Updates needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	///
	void edit(BufferView *, int, int, unsigned int) {}
	///
	EDITABLE editable() const{ return NOT_EDITABLE; }
	///
	bool display() const { return true; }
	///
	Inset::Code lyxCode() const;
	///
	string const getScreenLabel() const;
};

#endif
