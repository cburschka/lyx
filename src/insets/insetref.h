// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1997 LyX Team (this file was created this year)
 * 
 *======================================================*/

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
	~InsetRef();
        ///
        Inset * Clone() { return new InsetRef (getCommand(), master); }
	///
	Inset::Code LyxCode() const { return Inset::REF_CODE; }
	///
	void Edit(int, int);
	///
	unsigned char Editable() const {
		return 1;
	}
        ///
	bool Display() const { return false; }
	///
	string getScreenLabel() const;
	///
	InsetRef::Ref_Flags getFlag() { return flag; }
	///
	void setFlag(InsetRef::Ref_Flags f) { flag = f; }
        ///
        void gotoLabel();
	///
	int Latex(FILE * file, signed char fragile);
	///
	int Latex(string & file, signed char fragile);
	///
	int Linuxdoc(string & file);
	///
	int DocBook(string & file);
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
	master->gotoLabel(getContents());
    }
}
#endif
