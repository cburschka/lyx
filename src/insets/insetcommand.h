// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
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
	InsetCommandParams(string const & n,
			    string const & c = string(),
			    string const & o = string());
	///
	bool operator==(InsetCommandParams const &) const;
	///
	bool operator!=(InsetCommandParams const &) const;
	///
	void read(LyXLex &);
	/// Parse the command
	void scanCommand(string const &);
	///
	void write(std::ostream &) const;
	/// Build the complete LaTeX command
	string const getCommand() const;
	///
	string const & getCmdName() const { return cmdname; }
	///
	string const & getOptions() const { return options; }
	///
	string const & getContents() const { return contents; }
	///
	void setCmdName(string const & n) { cmdname = n; }
	///
	void setOptions(string const & o) { options = o; }
	///
	void setContents(string const & c) { contents = c; }
	///
	string const getAsString() const;
	///
	void setFromString(string const &);
private:
	///
	string cmdname;
	///
	string contents;
	///
	string options;
};


///
class InsetCommand : public InsetButton, boost::noncopyable {
public:
	///
	explicit
	InsetCommand(InsetCommandParams const &, bool same_id = false);
	///
	virtual ~InsetCommand() { hideDialog(); }
	///
	void write(Buffer const *, std::ostream & os) const
		{ p_.write(os); }
	///
	virtual void read(Buffer const *, LyXLex & lex)
		{ p_.read(lex); }
	/// Can remove one InsetBibKey is modified
	void scanCommand(string const & c) { p_.scanCommand(c); };
	///
	virtual int latex(Buffer const *, std::ostream &,
			  bool fragile, bool free_spc) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	virtual int linuxdoc(Buffer const *, std::ostream &) const;
	///
	virtual int docbook(Buffer const *, std::ostream &) const;
	///
	Inset::Code lyxCode() const { return Inset::NO_CODE; }

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
	SigC::Signal0<void> hideDialog;

private:
	///
	InsetCommandParams p_;
};

#endif
