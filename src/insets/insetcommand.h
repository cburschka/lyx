// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef INSET_LATEXCOMMAND_H
#define INSET_LATEXCOMMAND_H

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
	int ascent(Painter &, LyXFont const &) const;
	///
	int descent(Painter &, LyXFont const &) const;
	///
	int width(Painter &, LyXFont const &) const;
	///
	void draw(Painter &, LyXFont const &, int baseline, float & x) const;
	///
	void Write(ostream &) const;
	/// Parse the command.
	void scanCommand(string const & cmd);
	/// Will not be used when lyxf3
	void Read(LyXLex & lex);
	/// 
	virtual int Latex(ostream &, signed char fragile, bool free_spc) const;
	///
	virtual int Linuxdoc(ostream &) const;
	///
	virtual int DocBook(ostream &) const;
	///
	Inset * Clone() const;
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
	string const & getCmdName() const {
		return command;
	}
	///
	string const & getOptions() const {
		return options;
	}
	///
	string const & getContents() const {
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
