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
#include "insetcommandparams.h"
#include <boost/signals/signal0.hpp>
#include <boost/utility.hpp>

// Created by Alejandro 970222
/** Used to insert a LaTeX command automatically
 *
 * Similar to InsetLaTeX but having control of the basic structure of a
 *   LaTeX command: \name[options]{contents}.
 */

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
	virtual int docbook(Buffer const *, std::ostream &, bool) const;
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
	boost::signal0<void> hideDialog;

private:
	///
	InsetCommandParams p_;
};

#endif
