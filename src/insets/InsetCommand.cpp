/**
 * \file InsetCommand.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetCommand.h"

#include "Buffer.h"
#include "BufferView.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Lexer.h"
#include "MetricsInfo.h"

#include "support/debug.h"
#include "support/gettext.h"

#include "frontends/Application.h"

#include <sstream>

using namespace std;


namespace lyx {

// FIXME Would it now be possible to use the InsetCode in 
// place of the mailer name and recover that information?
InsetCommand::InsetCommand(InsetCommandParams const & p,
			   string const & mailer_name)
	: p_(p),
	  mailer_name_(mailer_name),
	  mouse_hover_(false)
{}


InsetCommand::~InsetCommand()
{
	if (!mailer_name_.empty())
		hideDialogs(mailer_name_, this);
}


void InsetCommand::metrics(MetricsInfo & mi, Dimension & dim) const
{
	button_.update(screenLabel(), editable() != NOT_EDITABLE);
	button_.metrics(mi, dim);
}


bool InsetCommand::setMouseHover(bool mouse_hover)
{
	mouse_hover_ = mouse_hover;
	return true;
}


void InsetCommand::draw(PainterInfo & pi, int x, int y) const
{
	button_.setRenderState(mouse_hover_);
	button_.draw(pi, x, y);
}


void InsetCommand::setParam(std::string const & name, docstring const & value)
{
	p_[name] = value;
}


docstring const & InsetCommand::getParam(std::string const & name) const
{
	return p_[name];
}


void InsetCommand::setParams(InsetCommandParams const & p)
{
	p_ = p;
	initView();
}


int InsetCommand::latex(odocstream & os, OutputParams const &) const
{
	os << getCommand();
	return 0;
}


int InsetCommand::plaintext(odocstream & os, OutputParams const &) const
{
	docstring const str = "[" + buffer().B_("LaTeX Command: ")
		+ from_utf8(getCmdName()) + "]";
	os << str;
	return str.size();
}


int InsetCommand::docbook(odocstream &, OutputParams const &) const
{
	return 0;
}


void InsetCommand::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY: {
		if (cmd.getArg(0) == "changetype") {
			p_.setCmdName(cmd.getArg(1));
			initView();
			break;
		}
		InsetCommandParams p(p_.code());
		InsetCommand::string2params(mailer_name_, to_utf8(cmd.argument()), p);
		if (p.getCmdName().empty())
			cur.noUpdate();
		else
			setParams(p);
		break;
	}

	case LFUN_INSET_DIALOG_UPDATE: {
		string const name = to_utf8(cmd.argument());
		cur.bv().updateDialog(name, params2string(name, params()));
		break;
	}

	case LFUN_MOUSE_RELEASE: {
		if (!cur.selection() && cmd.button() != mouse_button::button3)
			edit(cur, true);
		break;
	}

	default:
		Inset::doDispatch(cur, cmd);
		break;
	}

}


bool InsetCommand::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
	// suppress these
	case LFUN_ERT_INSERT:
		status.setEnabled(false);
		return true;
	// we handle these
	case LFUN_INSET_MODIFY:
		if (cmd.getArg(0) == "changetype") {
			string const newtype = cmd.getArg(1);
			status.setEnabled(p_.isCompatibleCommand(p_.code(), newtype));
			status.setOnOff(newtype == p_.getCmdName());
		} 
		status.setEnabled(true);
		return true;
	case LFUN_INSET_DIALOG_UPDATE:
		status.setEnabled(true);
		return true;
	// We can't suppress entire LFUN_INSET_TOGGLE as long as
	// LFUN_NEXT_INSET_TOGGLE is used for editation purposes.
	case LFUN_INSET_TOGGLE:
		status.setEnabled(cmd.argument() != "open" &&
				  cmd.argument() != "close" &&
				  cmd.argument() != "toggle");
		return true;
	default:
		return Inset::getStatus(cur, cmd, status);
	}
}


docstring InsetCommand::contextMenu(BufferView const &, int, int) const
{
	return from_ascii("context-") + from_ascii(mailer_name_);
}


void InsetCommand::edit(Cursor & cur, bool, EntryDirection)
{
	if (!mailer_name_.empty())
		cur.bv().showDialog(mailer_name_, params2string(mailer_name_, p_), this);
}


// FIXME This could take an InsetCode instead of a string
bool InsetCommand::string2params(string const & name, string const & in,
	InsetCommandParams & params)
{
	params.clear();
	if (in.empty())
		return false;
	istringstream data(in);
	Lexer lex;
	lex.setStream(data);
	lex.setContext("InsetCommand::string2params");
	lex >> name.c_str(); // check for name
	lex >> "CommandInset";
	params.read(lex);
	return true;
}


// FIXME This could take an InsetCode instead of a string
string InsetCommand::params2string(string const & name,
				  InsetCommandParams const & params)
{
	ostringstream data;
	data << name << ' ';
	params.write(data);
	data << "\\end_inset\n";
	return data.str();
}


} // namespace lyx
