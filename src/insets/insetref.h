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
class InsetRef : public InsetCommand {
public:
        ///
        enum Ref_Flags {
		///
		REF = 0,
		///
		PAGE_REF,
		///
		VREF,
		///
		VPAGE_REF,
		///
		PRETTY_REF,
		///
		REF_LAST = PRETTY_REF,
		///
		REF_FIRST = REF
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
	void Toggle();
        ///
        void gotoLabel();
	///
	int Latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const;
	///
	int Ascii(Buffer const *, std::ostream &) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
	///
	void Validate(LaTeXFeatures & features) const;
private:
	///
	void GenerateFlag();
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
