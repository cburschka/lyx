// -*- C++ -*-
/**
 * \file InsetCommand.h
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

#include "Inset.h"
#include "InsetCommandParams.h"
#include "RenderButton.h"
#include "MailInset.h"
#include "Cursor.h"


namespace lyx {


// Created by Alejandro 970222
/** Used to insert a LaTeX command automatically.
 */

///
class InsetCommand : public Inset {
public:
	///
	InsetCommand(InsetCommandParams const &, std::string const & mailer_name);
	///
	~InsetCommand();
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	Dimension const dimension(BufferView const &) const { return button_.dimension(); }
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(Buffer const &, std::ostream & os) const
		{ p_.write(os); }
	///
	virtual void read(Buffer const &, Lexer & lex)
		{ p_.read(lex); }
	///
	int latex(Buffer const &, odocstream &, OutputParams const &) const;
	///
	int plaintext(Buffer const &, odocstream &, OutputParams const &) const;
	///
	int docbook(Buffer const &, odocstream &, OutputParams const & runparams) const;
	///
	InsetCode lyxCode() const { return NO_CODE; }
	///
	InsetCommandParams const & params() const { return p_; }
	/// FIXME Remove
	docstring const getFirstNonOptParam() const { return p_.getFirstNonOptParam(); }
	/// Whether the button label should be recomputed.
	void refresh() { updateButtonLabel_ = true; }
	///
	void setParam(std::string const & name, docstring const & value)
	{
		updateButtonLabel_ = true;
		p_[name] = value;
	}
	///
	docstring const & getParam(std::string const & name) const
	{
		return p_[name];
	}
	///
	void edit(Cursor & cur, bool left);
	///
	RenderButton & button() const { return button_; }
	///
	bool setMouseHover(bool mouse_hover);
	/// Return parameter information for command cmdName.
	/// Not implemented here. Must be implemented in derived class.
	static CommandInfo const * findInfo(std::string const & cmdName);
	/// Return default command for this inset.
	/// Not implemented here. Must be implemented in derived class.
	static std::string defaultCommand();
	/// Whether this is a command this inset can represent.
	/// Not implemented here. Must be implemented in derived class.
	static bool isCompatibleCommand(std::string const & cmd);

protected:
	///
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;
	///
	docstring const getCommand() const { return p_.getCommand(); }
	///
	std::string const & getCmdName() const { return p_.getCmdName(); }
	///
	void setCmdName(std::string const & n)
	{
		updateButtonLabel_ = true;
		p_.setCmdName(n);
	}
	///
	void setParams(InsetCommandParams const &);
	/// This should provide the text for the button
	virtual docstring const getScreenLabel(Buffer const &) const = 0;

private:
	///
	InsetCommandParams p_;
	///
	std::string mailer_name_;
	/// changes color when mouse enters/leaves this inset
	bool mouse_hover_;
	///
	mutable bool updateButtonLabel_;
	///
	mutable RenderButton button_;
};


class InsetCommandMailer : public MailInset {
public:
	///
	InsetCommandMailer(std::string const & name, InsetCommand & inset);
	///
	virtual Inset & inset() const { return inset_; }
	///
	virtual std::string const & name() const { return name_; }
	///
	virtual std::string const inset2string(Buffer const &) const;
	/// returns true if params are successfully read
	static bool string2params(std::string const &, std::string const & name,
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



} // namespace lyx

#endif
