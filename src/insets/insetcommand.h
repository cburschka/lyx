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

#include "insetbutton.h"

// Created by Alejandro 970222
/** Used to insert a LaTeX command automatically
 *
 * Similar to InsetLaTeX but having control of the basic structure of a
 *   LaTeX command: \name[options]{contents}. 
 */
class InsetCommand: public InsetButton {
public:
	///
	InsetCommand();
	///
	explicit
	InsetCommand(string const & name, string const & arg = string(), 
		     string const & opt = string());
	///
	void Write(Buffer const *, std::ostream &) const;

	/// Parse the command.
	void scanCommand(string const & cmd);
	/// Will not be used when lyxf3
	void Read(Buffer const *, LyXLex & lex);
	/// 
	virtual int Latex(Buffer const *, std::ostream &,
			  bool fragile, bool free_spc) const;
	///
	int Ascii(Buffer const *, std::ostream &) const;
	///
	virtual int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	virtual int DocBook(Buffer const *, std::ostream &) const;
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
		return cmdname;
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
		cmdname = n;
	}
	///
	void setOptions(string const & o) {
		options = o;
	}
	///
	virtual void setContents(string const & c) {
		contents = c;
	}
	///
	void addContents(string const & c) {
		contents += c;
	}
private:
	///    
	string cmdname;
	///    
	string options;
	///    
	string contents;
};

#endif
