// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *          Copyright (C) 1996-1998 The LyX Team.
 *
 *======================================================*/

#ifndef _INSET_LATEXCOMMAND_H
#define _INSET_LATEXCOMMAND_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxinset.h"
#include "LString.h"

// Created by Alejandro 970222
/** Used to insert a LaTeX command automatically
 *
 * Similar to InsetLaTeX but having control of the basic structure of a
 *   LaTeX command: \name[options]{contents}. 
 */
class InsetCommand: public Inset {
public:
	///
	InsetCommand();
	///
	InsetCommand(string const & name, string const & arg = string(), 
		     string const & opt = string());
	///
	~InsetCommand();
	///
	int Ascent(LyXFont const &font) const;
	///
	int Descent(LyXFont const &font) const;
	///
	int Width(LyXFont const &font) const;
	///
	void Draw(LyXFont, LyXScreen &scr, int baseline, float &x);
	///
	void Write(FILE *file);
	/// Parse the command.
	void scanCommand(string const &cmd);
	/// Will not be used when lyxf3
	void Read(LyXLex &lex);
	/// 
	virtual int Latex(FILE *file, signed char fragile);
	///
	virtual int Latex(string &file, signed char fragile);
	///
	virtual int Linuxdoc(string &file);
	///
	virtual int DocBook(string &file);
	///
	Inset* Clone();
	///  
	Inset::Code LyxCode() const
	{
		return Inset::NO_CODE;
	}
	
	/** Get the label that appears at screen.
	  
         I thought it was enough to eliminate the argument to avoid
         confusion with lyxinset::getLabel(int), but I've seen that
         it wasn't. I hope you never confuse again both methods.  (ale)
	 */
	virtual string getScreenLabel() const
	{
		return getCommand();
	}
	
	/// Build the complete LaTeX command
	string getCommand() const;
	///
	string const &getCmdName() const {
		return command;
	}
	///
	string const &getOptions() const {
		return options;
	}
	///
	string const &getContents() const {
		return contents;
	}
	///
	void setCmdName(string const & n) {
		command = n;
	}
	///
	void setOptions(string const & o) {
		options = o;
	}
	///
	virtual void setContents(string const & c) {
		contents = c;
	}
protected:
	///    
	string command;
	///    
	string options;
	///    
	string contents;
};

#endif
