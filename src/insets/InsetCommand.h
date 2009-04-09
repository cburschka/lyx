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

#ifndef INSETCOMMAND_H
#define INSETCOMMAND_H

#include "Inset.h"
#include "InsetCommandParams.h"
#include "RenderButton.h"
#include "Cursor.h"


namespace lyx {


/////////////////////////////////////////////////////////////////////////
//
// InsetCommand
//
/////////////////////////////////////////////////////////////////////////

// Created by Alejandro 970222
// Used to insert a LaTeX command automatically.

class InsetCommand : public Inset
{
public:
	///
	InsetCommand(InsetCommandParams const &, std::string const & mailer_name);
	///
	~InsetCommand();

	/// returns true if params are successfully read
	static bool string2params(std::string const &, std::string const & name,
				  InsetCommandParams &);
	///
	static std::string params2string(std::string const & name,
					       InsetCommandParams const &);
	///
	InsetCommandParams const & params() const { return p_; }
	///
	void setParams(InsetCommandParams const &);
	///
	void setParam(std::string const & name, docstring const & value);
	///
	docstring const & getParam(std::string const & name) const;
	/// FIXME Remove
	docstring const getFirstNonOptParam() const { return p_.getFirstNonOptParam(); }
	/// update label and references.
	virtual void updateCommand(docstring const &, bool) {}

protected:
	///
	void write(std::ostream & os) const { p_.write(os); }
	///
	void read(Lexer & lex) { p_.read(lex); }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;
	///
	docstring const getCommand() const { return p_.getCommand(); }
	///
	std::string const & getCmdName() const { return p_.getCmdName(); }
	///
	void setCmdName(std::string const & n) { p_.setCmdName(n); }

private:
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	Dimension const dimension(BufferView const &) const { return button_.dimension(); }
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const & runparams) const;
	///
	InsetCode lyxCode() const { return NO_CODE; }
	///
	void edit(Cursor & cur, bool front, 
		EntryDirection entry_from = ENTRY_DIRECTION_IGNORE);
	///
	RenderButton & button() const { return button_; }
	///
	bool setMouseHover(bool mouse_hover);
	/// Return parameter information for command cmdName.
	/// Not implemented here. Must be implemented in derived class.
	static ParamInfo const & findInfo(std::string const & cmdName);
	/// Return default command for this inset.
	/// Not implemented here. Must be implemented in derived class.
	static std::string defaultCommand();
	/// Whether this is a command this inset can represent.
	/// Not implemented here. Must be implemented in derived class.
	static bool isCompatibleCommand(std::string const & cmd);
	///
	docstring contextMenu(BufferView const & bv, int x, int y) const;
	/// This should provide the text for the button
	virtual docstring screenLabel() const = 0;
	///
	bool showInsetDialog(BufferView * bv) const;

private:
	///
	InsetCommandParams p_;
	///
	std::string mailer_name_;
	/// changes color when mouse enters/leaves this inset
	bool mouse_hover_;
	///
	mutable RenderButton button_;
};


} // namespace lyx

#endif
