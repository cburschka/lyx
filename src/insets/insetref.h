// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1997 LyX Team (this file was created this year)
 * 
 * ====================================================== */

#ifndef INSET_REF_H
#define INSET_REF_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"
#include "buffer.h"

struct LaTeXFeatures;

/** The reference inset  
 */
class InsetRef: public InsetCommand {
public:
        ///
        enum Ref_Flags {
		///
		REF,
		///
		PAGE_REF
	};
	
	///
	InsetRef() : InsetCommand("ref") { flag = InsetRef::REF; }
	///
	InsetRef(string const &, Buffer *);
	///
	InsetRef(InsetCommand const &, Buffer *);
        ///
        Inset * Clone() const {
		return new InsetRef (getCommand(), master);
	}
	///
	Inset::Code LyxCode() const { return Inset::REF_CODE; }
	///
	void Edit(BufferView *, int, int, unsigned int);
	///
	EDITABLE Editable() const {
		return IS_EDITABLE;
	}
        ///
	bool display() const { return false; }
	///
	string getScreenLabel() const;
	///
	InsetRef::Ref_Flags getFlag() { return flag; }
	///
	void setFlag(InsetRef::Ref_Flags f) { flag = f; }
        ///
        void gotoLabel();
	///
	int Latex(ostream &, signed char fragile, bool free_spc) const;
	///
	int Linuxdoc(ostream &) const;
	///
	int DocBook(ostream &) const;
private:
	/// This function escapes 8-bit characters
	string escape(string const &) const;
	///
        Ref_Flags flag;
        ///
	Buffer * master;
};


inline
void InsetRef::gotoLabel()
{
    if (master) {
	master->getUser()->gotoLabel(getContents());
    }
}
#endif
