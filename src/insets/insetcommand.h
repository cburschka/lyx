// -*- C++ -*-
/**
 * \file insetcommand.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSET_LATEXCOMMAND_H
#define INSET_LATEXCOMMAND_H


#include "insetbutton.h"
#include "insetcommandparams.h"
#include "mailinset.h"
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
	void write(Buffer const *, std::ostream & os) const
		{ p_.write(os); }
	///
	virtual void read(Buffer const *, LyXLex & lex)
		{ p_.read(lex); }
	/// Can remove one InsetBibKey is modified
	void scanCommand(string const & c) { p_.scanCommand(c); };
	///
	virtual int latex(Buffer const *, std::ostream &,
			  LatexRunParams const &) const;
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
	virtual dispatch_result localDispatch(FuncRequest const & cmd);

private:
	///
	InsetCommandParams p_;
};


class InsetCommandMailer : public MailInset {
public:
	///
	InsetCommandMailer(string const & name, InsetCommand & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual string const & name() const { return name_; }
	///
	virtual string const inset2string() const;
	///
	static void string2params(string const &, InsetCommandParams &);
	///
	static string const params2string(string const & name,
					  InsetCommandParams const &);
private:
	///
	string const name_;
	///
	InsetCommand & inset_;
};


#endif
