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
#include "support/utility.hpp"

struct LaTeXFeatures;

/** Used to insert index labels  
  */
class InsetIndex : public InsetCommand, public noncopyable {
public:
	///
	InsetIndex(InsetCommandParams const &);
	///
	Inset * Clone() const { return new InsetIndex(params());}
	///
	string getScreenLabel() const;
	///
	EDITABLE Editable() const { return IS_EDITABLE; }
	///
	void Edit(BufferView *, int, int, unsigned int);
};


class InsetPrintIndex : public InsetCommand, public noncopyable {
public:
	///
	InsetPrintIndex(InsetCommandParams const &);
	///
	Inset * Clone() const { return new InsetPrintIndex(params());}
	/// Updates needed features for this inset.
	void Validate(LaTeXFeatures & features) const;
	///
	void Edit(BufferView *, int, int, unsigned int) {}
	///
	EDITABLE Editable() const{ return NOT_EDITABLE; }
	///
	bool display() const { return true; }
	///
	Inset::Code LyxCode() const;
	///
	string getScreenLabel() const;
};

#endif
