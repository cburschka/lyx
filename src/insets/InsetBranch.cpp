/**
 * \file InsetBranch.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetBranch.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "BranchList.h"
#include "ColorSet.h"
#include "Counters.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Lexer.h"
#include "OutputParams.h"
#include "TextClass.h"
#include "TocBackend.h"

#include "support/debug.h"
#include "support/gettext.h"

#include "frontends/Application.h"

#include <sstream>

using namespace std;


namespace lyx {

InsetBranch::InsetBranch(Buffer const & buf, InsetBranchParams const & params)
	: InsetCollapsable(buf), params_(params)
{
	// override the default for InsetCollapsable, which is to
	// use the plain layout.
	DocumentClass const & dc = buf.params().documentClass();
	paragraphs().back().setDefaultLayout(dc);
}


InsetBranch::~InsetBranch()
{
	hideDialogs("branch", this);
}


docstring InsetBranch::editMessage() const
{
	return _("Opened Branch Inset");
}


void InsetBranch::write(ostream & os) const
{
	params_.write(os);
	InsetCollapsable::write(os);
}


void InsetBranch::read(Lexer & lex)
{
	params_.read(lex);
	InsetCollapsable::read(lex);
}


docstring InsetBranch::toolTip(BufferView const &, int, int) const
{
	return _("Branch: ") + params_.branch;
}


docstring const InsetBranch::buttonLabel(BufferView const & bv) const
{
	docstring s = _("Branch: ") + params_.branch;
	if (!params_.branch.empty()) {
		// FIXME UNICODE
		ColorCode c = lcolor.getFromLyXName(to_utf8(params_.branch));
		if (c == Color_none)
			s = _("Undef: ") + s;
	}
	if (decoration() == InsetLayout::CLASSIC)
		return isOpen(bv) ? s : getNewLabel(s);
	else
		return params_.branch + ": " + getNewLabel(s);
}


ColorCode InsetBranch::backgroundColor() const
{
	if (params_.branch.empty())
		return Inset::backgroundColor();
	// FIXME UNICODE
	ColorCode c = lcolor.getFromLyXName(to_utf8(params_.branch));
	if (c == Color_none)
		c = Color_error;
	return c;
}


bool InsetBranch::showInsetDialog(BufferView * bv) const
{
	bv->showDialog("branch", params2string(params()),
			const_cast<InsetBranch *>(this));
	return true;
}


void InsetBranch::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY: {
		InsetBranchParams params;
		InsetBranch::string2params(to_utf8(cmd.argument()), params);
		params_.branch = params.branch;
		setLayout(cur.buffer()->params());
		break;
	}

	case LFUN_MOUSE_PRESS:
		if (cmd.button() != mouse_button::button3)
			InsetCollapsable::doDispatch(cur, cmd);
		else
			cur.undispatched();
		break;

	case LFUN_INSET_DIALOG_UPDATE:
		cur.bv().updateDialog("branch", params2string(params()));
		break;

	case LFUN_INSET_TOGGLE:
		if (cmd.argument() == "assign") {
			// The branch inset uses "assign".
			if (isBranchSelected()) {
				if (status(cur.bv()) != Open)
					setStatus(cur, Open);
				else
					cur.undispatched();
			} else {
				if (status(cur.bv()) != Collapsed)
					setStatus(cur, Collapsed);
				else
					cur.undispatched();
			}
		}
		else
			InsetCollapsable::doDispatch(cur, cmd);
		break;

	default:
		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}
}


bool InsetBranch::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {
	case LFUN_INSET_MODIFY:
	case LFUN_INSET_DIALOG_UPDATE:
		flag.setEnabled(true);
		break;

	case LFUN_INSET_TOGGLE:
		if (cmd.argument() == "open" || cmd.argument() == "close" ||
		    cmd.argument() == "toggle")
			flag.setEnabled(true);
		else if (cmd.argument() == "assign" || cmd.argument().empty()) {
			if (isBranchSelected())
				flag.setEnabled(status(cur.bv()) != Open);
			else
				flag.setEnabled(status(cur.bv()) != Collapsed);
		} else
			flag.setEnabled(true);
		break;

	default:
		return InsetCollapsable::getStatus(cur, cmd, flag);
	}
	return true;
}


bool InsetBranch::isBranchSelected() const
{
	Buffer const & realbuffer = *buffer().masterBuffer();
	BranchList const & branchlist = realbuffer.params().branchlist();
	Branch const * ourBranch = branchlist.find(params_.branch);
	if (!ourBranch)
		return false;
	return ourBranch->isSelected();
}


int InsetBranch::latex(odocstream & os, OutputParams const & runparams) const
{
	return isBranchSelected() ?  InsetText::latex(os, runparams) : 0;
}


int InsetBranch::plaintext(odocstream & os,
			   OutputParams const & runparams) const
{
	if (!isBranchSelected())
		return 0;

	os << '[' << buffer().B_("branch") << ' ' << params_.branch << ":\n";
	InsetText::plaintext(os, runparams);
	os << "\n]";

	return PLAINTEXT_NEWLINE + 1; // one char on a separate line
}


int InsetBranch::docbook(odocstream & os,
			 OutputParams const & runparams) const
{
	return isBranchSelected() ?  InsetText::docbook(os, runparams) : 0;
}


void InsetBranch::tocString(odocstream & os) const
{
	if (isBranchSelected())
		InsetCollapsable::tocString(os);
}


void InsetBranch::validate(LaTeXFeatures & features) const
{
	if (isBranchSelected())
		InsetCollapsable::validate(features);
}


bool InsetBranch::isMacroScope() const 
{
	// Its own scope if not selected by buffer
	return !isBranchSelected();
}


string InsetBranch::params2string(InsetBranchParams const & params)
{
	ostringstream data;
	data << "branch" << ' ';
	params.write(data);
	return data.str();
}


void InsetBranch::string2params(string const & in, InsetBranchParams & params)
{
	params = InsetBranchParams();
	if (in.empty())
		return;

	istringstream data(in);
	Lexer lex;
	lex.setStream(data);
	lex.setContext("InsetBranch::string2params");
	lex >> "branch" >> "Branch";
	params.read(lex);
}


void InsetBranch::addToToc(DocIterator const & cpit)
{
	DocIterator pit = cpit;
	pit.push_back(CursorSlice(*this));

	Toc & toc = buffer().tocBackend().toc("branch");
	docstring const str = params_.branch + ": " + text().getPar(0).asString();
	toc.push_back(TocItem(pit, 0, str));
	// Proceed with the rest of the inset.
	InsetCollapsable::addToToc(cpit);
}


void InsetBranchParams::write(ostream & os) const
{
	os << "Branch " << to_utf8(branch) << '\n';
}


void InsetBranchParams::read(Lexer & lex)
{
	lex.eatLine();
	branch = lex.getDocString();
}

} // namespace lyx
