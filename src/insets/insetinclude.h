// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1997 LyX Team (this file was created this year)
 * 
 * ====================================================== */

#ifndef INSET_INCLUDE_H
#define INSET_INCLUDE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"

class Buffer;
struct LaTeXFeatures;

// Created by AAS 970521

/**  Used to include files
 */
class InsetInclude: public InsetCommand {
public:
	///
	InsetInclude(InsetCommandParams const &, Buffer const &);
	///
	~InsetInclude();
        ///
        Inset * Clone(Buffer const &) const;
	///
	Inset::Code LyxCode() const { return Inset::INCLUDE_CODE; }
	/// This returns the list of labels on the child buffer
	std::vector<string> const getLabelList() const;
	/// This returns the list of bibkeys on the child buffer
	std::vector< std::pair<string,string> > const getKeys() const;
	///
	void Edit(BufferView *, int x, int y, unsigned int button);
	///
	EDITABLE Editable() const
	{
		return IS_EDITABLE;
	}
        /// With lyx3 we won't overload these 3 methods
        void Write(Buffer const *, std::ostream &) const;
        ///
	void Read(Buffer const *, LyXLex &);
	/// 
	int Latex(Buffer const *, std::ostream &, bool fragile, bool free_spc) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
	///
	void Validate(LaTeXFeatures &) const;
	
        /** Input inserts anything inside a paragraph.
	    Display can give some visual feedback
	*/
	bool display() const;
	///
	string const getScreenLabel() const;
        ///
        string const getMasterFilename() const;
        ///
        string const getFileName() const;

        ///  In "input" mode uses \input instead of \include.
	bool isInput() const { return flag == InsetInclude::INPUT; }
        ///  If this is true, the child file shouldn't be loaded by lyx
	bool isNoLoad() const { return noload; }

        /**  A verbatim file shouldn't be loaded by LyX
	 *  No need to generate LaTeX code of a verbatim file
	 */ 
	bool isVerb() const;
	///
	bool isVerbVisibleSpace() const {
		return flag == InsetInclude::VERBAST;
	}
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
	Buffer const * master;
	///
	string include_label;
};


inline 
bool InsetInclude::isVerb() const
{
	return flag == InsetInclude::VERB || flag == InsetInclude::VERBAST; 
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
