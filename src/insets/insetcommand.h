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
#include <sigc++/signal_system.h>
#include <boost/utility.hpp>

#ifdef SIGC_CXX_NAMESPACES
using SigC::Signal0;
#endif

// Created by Alejandro 970222
/** Used to insert a LaTeX command automatically
 *
 * Similar to InsetLaTeX but having control of the basic structure of a
 *   LaTeX command: \name[options]{contents}. 
 */
class InsetCommandParams {
public:
	///
	InsetCommandParams();
	///
	explicit
	InsetCommandParams( string const & n,
			    string const & c = string(),
			    string const & o = string());
	///
	bool operator==(InsetCommandParams const &) const;
	///
	bool operator!=(InsetCommandParams const &) const;
	///
	void Read(LyXLex &);
	/// Parse the command
	void scanCommand(string const &);
	///
	void Write(std::ostream &) const;
	/// Build the complete LaTeX command
	string const getCommand() const;
	///
	string const & getCmdName() const { return cmdname; }
	///
	string const & getOptions() const { return options; }
	///
	string const & getContents() const { return contents; }
	///
	void setCmdName( string const & n) { cmdname = n; }
	///
	void setOptions(string const & o) { options = o; }
	///
	void setContents(string const & c) { contents = c; }
	///
	string const getAsString() const;
	///
	void setFromString( string const & );
private:
	///    
	string cmdname;
	///    
	string contents;
	///    
	string options;
};


///
class InsetCommand : public InsetButton, public noncopyable {
public:
	///
	explicit
	InsetCommand(InsetCommandParams const &);
	///
	virtual ~InsetCommand() { hide(); }
	///
	void Write(Buffer const *, std::ostream & os) const
		{ p_.Write( os ); }
	///
	virtual void Read(Buffer const *, LyXLex & lex)
		{ p_.Read( lex ); }
	/// Can remove one InsetBibKey is modified
	void scanCommand(string const & c) { p_.scanCommand( c ); };
	/// 
	virtual int Latex(Buffer const *, std::ostream &,
			  bool fragile, bool free_spc) const;
	///
	int Ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	virtual int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	virtual int DocBook(Buffer const *, std::ostream &) const;
	///
	Inset::Code LyxCode() const { return Inset::NO_CODE; }
	
	/** Get the label that appears at screen.
	  
         I thought it was enough to eliminate the argument to avoid
         confusion with lyxinset::getLabel(int), but I've seen that
         it wasn't. I hope you never confuse again both methods.  (ale)
	 */
	virtual string const getScreenLabel() const = 0;
	///
	string const getCommand() const { return p_.getCommand(); }
	///
	string const & getCmdName() const { return p_.getCmdName(); }
	///
	string const & getOptions() const { return p_.getOptions(); }
	///
	string const & getContents() const { return p_.getContents(); }
	///
	void setCmdName(string const & n) { p_.setCmdName(n); }
	///
	void setOptions(string const & o) { p_.setOptions(o); }
	///
	void setContents(string const & c) { p_.setContents(c); }
	///
	InsetCommandParams const & params() const { return p_; }
	///
	void setParams(InsetCommandParams const &);
	///
	Signal0<void> hide;
private:
	///
	InsetCommandParams p_;
};

#endif
