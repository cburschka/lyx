// -*- C++ -*-
/**
 * \file insetcommand.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_LATEXCOMMAND_H
#define INSET_LATEXCOMMAND_H


#include "inset.h"
#include "insetcommandparams.h"
#include "render_button.h"
#include "mailinset.h"

// Created by Alejandro 970222
/** Used to insert a LaTeX command automatically
 *
 * Similar to InsetLaTeX but having control of the basic structure of a
 *   LaTeX command: \name[options]{contents}.
 */

///
class InsetCommand : public InsetOld {
public:
	///
	explicit
	InsetCommand(InsetCommandParams const &);
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(Buffer const &, std::ostream & os) const
		{ p_.write(os); }
	///
	virtual void read(Buffer const &, LyXLex & lex)
		{ p_.read(lex); }
	/// Can remove one InsetBibKey is modified
	void scanCommand(std::string const & c) { p_.scanCommand(c); };
	///
	virtual int latex(Buffer const &, std::ostream &,
			  LatexRunParams const &) const;
	///
	int ascii(Buffer const &, std::ostream &, int linelen) const;
	///
	virtual int linuxdoc(Buffer const &, std::ostream &) const;
	///
	virtual int docbook(Buffer const &, std::ostream &, bool) const;
	///
	InsetOld::Code lyxCode() const { return InsetOld::NO_CODE; }
	///
	virtual bool display() const { return true; }
	
	///
	InsetCommandParams const & params() const { return p_; }
	///
	std::string const & getContents() const { return p_.getContents(); }
	///
	void setContents(std::string const & c) { p_.setContents(c); }
	///
	std::string const & getOptions() const { return p_.getOptions(); }
	///
	RenderButton & button() const { return button_; }

protected:
	///
	virtual
	dispatch_result
	priv_dispatch(FuncRequest const & cmd, idx_type &, pos_type &);
	///
	std::string const getCommand() const { return p_.getCommand(); }
	///
	std::string const & getCmdName() const { return p_.getCmdName(); }
	///
	void setCmdName(std::string const & n) { p_.setCmdName(n); }
	///
	void setOptions(std::string const & o) { p_.setOptions(o); }
	///
	void setParams(InsetCommandParams const &);
	/// This should provide the text for the button
	virtual std::string const getScreenLabel(Buffer const &) const = 0;

private:
	///
	InsetCommandParams p_;
	mutable bool set_label_;
	mutable RenderButton button_;
};


class InsetCommandMailer : public MailInset {
public:
	///
	InsetCommandMailer(std::string const & name, InsetCommand & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual std::string const & name() const { return name_; }
	///
	virtual std::string const inset2string(Buffer const &) const;
	///
	static void string2params(std::string const &, InsetCommandParams &);
	///
	static std::string const params2string(std::string const & name,
					  InsetCommandParams const &);
private:
	///
	std::string const name_;
	///
	InsetCommand & inset_;
};


#endif
