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
#include "cursor.h"


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
	InsetCommand(InsetCommandParams const &, std::string const & mailer_name);
	///
	~InsetCommand();
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
			  OutputParams const &) const;
	///
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	virtual int linuxdoc(Buffer const &, std::ostream &,
			     OutputParams const &) const;
	///
	virtual int docbook(Buffer const &, std::ostream &,
			    OutputParams const & runparams) const;
	///
	InsetOld::Code lyxCode() const { return InsetOld::NO_CODE; }

	///
	InsetCommandParams const & params() const { return p_; }
	///
	std::string const & getContents() const { return p_.getContents(); }
	///
	void setContents(std::string const & c) { p_.setContents(c); }
	///
	std::string const & getOptions() const { return p_.getOptions(); }
	///
	std::string const & getSecOptions() const { return p_.getSecOptions(); }
	///
	RenderButton & button() const { return button_; }

protected:
	///
	void priv_dispatch(LCursor & cur, FuncRequest & cmd);
	///
	std::string const getCommand() const { return p_.getCommand(); }
	///
	std::string const & getCmdName() const { return p_.getCmdName(); }
	///
	void setCmdName(std::string const & n) { p_.setCmdName(n); }
	///
	void setOptions(std::string const & o) { p_.setOptions(o); }
	///
	void setSecOptions(std::string const & s) { p_.setSecOptions(s); }
	///
	void setParams(InsetCommandParams const &);
	/// This should provide the text for the button
	virtual std::string const getScreenLabel(Buffer const &) const = 0;

private:
	///
	InsetCommandParams p_;
	std::string mailer_name_;
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
	static void string2params(std::string const &, std::string const & name,
				  InsetCommandParams &);
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
