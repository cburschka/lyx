// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *          Copyright (C) 1995-1998 The LyX Team.
 * 
 *======================================================*/

#ifndef _INSET_BIB_H
#define _INSET_BIB_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"

// Created by Alejandro 970222


/** Used to insert citations  
 */
class InsetCitation: public InsetCommand {
public:
	///
	InsetCitation(): InsetCommand("cite") { }
	///
	InsetCitation(LString const & key, LString const & note=LString());
        ///
	~InsetCitation();
        ///
        Inset* Clone() { return new InsetCitation(contents, options); }
    	///
	LString getScreenLabel()const;
        ///
	void Edit(int, int);
        ///
	unsigned char Editable() const {
		return 1;
	}
};


/** Used to insert bibitem's information (key and label)
  
  Must be automatically inserted as the first object in a
  bibliography paragraph. 
  */
class InsetBibKey: public InsetCommand {
public:
	///
	InsetBibKey(): InsetCommand("bibitem") { counter = 1; }
	///
	InsetBibKey(LString const & key, LString const & label=LString());
	///
	InsetBibKey(InsetBibKey const*);
	///
	~InsetBibKey();
	///
        Inset* Clone() { return new InsetBibKey(this); }
	/// Currently \bibitem is used as a LyX2.x command, so we need this method.
        void Write(FILE *);
	///
	virtual LString getScreenLabel() const;
        ///
	void Edit(int, int);
	///
	unsigned char Editable() const {
		return 1;
	}
	/// A user can't neither insert nor delete this inset
	bool Deletable() const {
		return false;
	}
        ///
        void setCounter(int);
        ///
        int  getCounter() const { return counter; }

 private:
	///
        int counter;
};


/** Used to insert BibTeX's information 
  */
class InsetBibtex: public InsetCommand {
public:
	/// 
	InsetBibtex(): InsetCommand("BibTeX") { owner = NULL; }
	///
	InsetBibtex(LString const & dbase, LString const & style,
		    Buffer *);
        ///
        ~InsetBibtex();
        ///
	Inset* Clone() { return new InsetBibtex(contents, options, NULL); }
	///  
	Inset::Code LyxCode() const
	{
		return Inset::BIBTEX_CODE;
	}
	///
	LString getScreenLabel() const;
	///
	void Edit(int, int);
	/// 
	int Latex(FILE *, signed char);
	///
	int Latex(LString &file, signed char fragile);
	///
	LString getKeys();
	///
	unsigned char Editable() const {
		return 1;
	}
        ///
        bool addDatabase(LString const&);
        ///
        bool delDatabase(LString const&);
	///
	bool Display() const { return true; }    
private:
	///
	Buffer *owner;
};


#endif
