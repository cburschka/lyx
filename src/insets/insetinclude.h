// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1997 LyX Team (this file was created this year)
 * 
 * ====================================================== */

#ifndef INSET_INCLUDE_H
#define INSET_INCLUDE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"
#include "buffer.h"
#include "support/filetools.h"

struct LaTeXFeatures;

// Created by AAS 970521

/**  Used to include files
 */
class InsetInclude: public InsetCommand {
public:
	///
	InsetInclude(): InsetCommand("include")
	{
		flag = InsetInclude::INCLUDE;
	}
	///
	InsetInclude(string const &,  Buffer *);
	///
	~InsetInclude();
        ///
        InsetInclude * Clone() const;
	///
	Inset::Code LyxCode() const { return Inset::INCLUDE_CODE; }
	/// This is 1 if the childs have labels, 0 otherwise
	int GetNumberOfLabels() const;
	/// This returns the list of labels on the child buffer
	string getLabel(int) const;
	/// This returns the list of bibkeys on the child buffer
	string getKeys() const;
	///
	void Edit(int, int);
	///
	unsigned char Editable() const
	{
		return 1;
	}
        /// With lyx3 we won't overload these 3 methods
        void Write(FILE *);
        ///
	void Read(LyXLex &);
	/// 
	int Latex(FILE * file, signed char fragile);
	///
	int Latex(string & file, signed char fragile);
	
	///
	void Validate(LaTeXFeatures &) const;
	
        /// Input inserts anything inside a paragraph, Display can give some visual feedback 
	bool display() const { return !(isInput()); }
	///
	string getScreenLabel() const;
	///
	void setContents(string const & c) {
		InsetCommand::setContents(c);
		filename = MakeAbsPath(contents, 
				       OnlyPath(getMasterFilename())); 
	}
        ///
        void setFilename(string const & n) { setContents(n); }
        ///
        string getMasterFilename() const { return master->getFileName(); }
        ///
        string getFileName() const { 
		return filename;
	}
        ///  In "input" mode uses \input instead of \include.
	bool isInput() const { return flag == InsetInclude::INPUT; }
        ///  If this is true, the child file shouldn't be loaded by lyx
	bool isNoLoad() const { return noload; }

        /**  A verbatim file shouldn't be loaded by LyX
	 *  No need to generate LaTeX code of a verbatim file
	 */ 
	bool isVerb() const;
	///
	bool isVerbVisibleSpace() const { return flag == InsetInclude::VERBAST;}
        ///  
	bool isInclude() const { return flag == InsetInclude::INCLUDE;}
        ///  
	void setInput();
        ///  
	void setNoLoad(bool);
        ///  
	void setInclude();
        ///  
	void setVerb();
	///
	void setVisibleSpace(bool b);
	/// return true if the file is or got loaded.
	bool loadIfNeeded() const;
private:
        ///
        enum Include_Flags {
		///
		INCLUDE= 0,
		///
		VERB = 1,
		///
		INPUT = 2,
		///
		VERBAST = 3
	};
	
	///
	bool noload;
	///
        int flag;
        ///
	Buffer * master;
	///
	string filename;
};


inline 
bool InsetInclude::isVerb() const
{
    return (bool)(flag == InsetInclude::VERB || flag == InsetInclude::VERBAST); 
}


inline
void InsetInclude::setInput()
{
	if (!isInput()) {
	    flag = InsetInclude::INPUT;
	    setCmdName("input");
	}
}


inline
void InsetInclude::setNoLoad(bool b)
{ 
		noload = b;
}


inline
void InsetInclude::setInclude()
{
	if (!isInclude()) {
	    flag = InsetInclude::INCLUDE;
	    setCmdName("include");
	}
}


inline
void InsetInclude::setVerb()
{ 
	if (!isVerb()) {
	    flag = InsetInclude::VERB;
	    setCmdName("verbatiminput");
	}
}


inline
void InsetInclude::setVisibleSpace(bool b)
{
        if (b && flag == InsetInclude::VERB) {
	    setCmdName("verbatiminput*");
	    flag = InsetInclude::VERBAST;
	} else if (!b && flag == InsetInclude::VERBAST) {
	    setCmdName("verbatiminput");
	    flag = InsetInclude::VERB;
	}
}
#endif
