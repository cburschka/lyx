// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *          Copyright (C) 1995-1999 The LyX Team.
 * 
 * ====================================================== */

#ifndef INSET_BIB_H
#define INSET_BIB_H

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
	InsetCitation(): InsetCommand("cite") {}
	///
	InsetCitation(string const & key, string const & note = string());
        ///
	~InsetCitation();
        ///
        Inset * Clone() const {
		return new InsetCitation(contents, options);
	}
    	///
	string getScreenLabel()const;
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
	InsetBibKey() : InsetCommand("bibitem") { counter = 1; }
	///
	InsetBibKey(string const & key, string const & label = string());
	///
	InsetBibKey(InsetBibKey const *);
	///
	~InsetBibKey();
	///
        Inset * Clone() const { return new InsetBibKey(this); }
	/// Currently \bibitem is used as a LyX2.x command, so we need this method.
        void Write(FILE *);
	///
	virtual string getScreenLabel() const;
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
	InsetBibtex(): InsetCommand("BibTeX") { owner = 0; }
	///
	InsetBibtex(string const & dbase, string const & style,
		    Buffer *);
        ///
	Inset * Clone() const {
		return new InsetBibtex(contents, options, 0);
	}
	///  
	Inset::Code LyxCode() const
	{
		return Inset::BIBTEX_CODE;
	}
	///
	string getScreenLabel() const;
	///
	void Edit(int, int);
	/// 
	int Latex(FILE *, signed char);
	///
	int Latex(string & file, signed char fragile);
	///
	string getKeys();
	///
	unsigned char Editable() const {
		return 1;
	}
        ///
        bool addDatabase(string const &);
        ///
        bool delDatabase(string const &);
	///
	bool display() const { return true; }    
private:
	///
	Buffer * owner;
};

#endif
