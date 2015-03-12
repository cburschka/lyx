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
#include "LyX.h"
#include "OutputParams.h"
#include "output_xhtml.h"
#include "TextClass.h"
#include "TocBackend.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include "frontends/alert.h"
#include "frontends/Application.h"

#include <sstream>

using namespace std;


namespace lyx {

InsetBranch::InsetBranch(Buffer * buf, InsetBranchParams const & params)
	: InsetCollapsable(buf, InsetText::DefaultLayout), params_(params)
{}


void InsetBranch::write(ostream & os) const
{
	os << "Branch ";
	params_.write(os);
	os << '\n';
	InsetCollapsable::write(os);
}


void InsetBranch::read(Lexer & lex)
{
	params_.read(lex);
	InsetCollapsable::read(lex);
}


docstring InsetBranch::toolTip(BufferView const & bv, int, int) const
{
	docstring const masterstatus = isBranchSelected() ?
		_("active") : _("non-active");
	docstring const childstatus = isBranchSelected(true) ?
		_("active") : _("non-active");
	docstring const status = (masterstatus == childstatus) ?
		masterstatus :
		support::bformat(_("master: %1$s, child: %2$s"),
						 masterstatus, childstatus);
	docstring const heading = 
		support::bformat(_("Branch (%1$s): %2$s"), status, params_.branch);
	if (isOpen(bv))
		return heading;
	return toolTipText(heading + from_ascii("\n"));
}


docstring const InsetBranch::buttonLabel(BufferView const & bv) const
{
	docstring s = _("Branch: ") + params_.branch;
	Buffer const & realbuffer = *buffer().masterBuffer();
	BranchList const & branchlist = realbuffer.params().branchlist();
	bool const inmaster = branchlist.find(params_.branch);
	bool const inchild = buffer().params().branchlist().find(params_.branch);
	if (!inmaster && inchild)
		s = _("Branch (child only): ") + params_.branch;
	else if (inmaster && !inchild)
		s = _("Branch (master only): ") + params_.branch;
	else if (!inmaster)
		s = _("Branch (undefined): ") + params_.branch;
	if (!params_.branch.empty()) {
		// FIXME UNICODE
		ColorCode c = lcolor.getFromLyXName(to_utf8(params_.branch));
		if (c == Color_none)
			s = _("Undef: ") + s;
	}
	bool const master_selected = isBranchSelected();
	bool const child_selected = isBranchSelected(true);
	docstring symb = docstring(1, char_type(master_selected ? 0x2714 : 0x2716));
	if (inchild && master_selected != child_selected)
		symb += char_type(child_selected ? 0x2714 : 0x2716);
	if (decoration() == InsetLayout::CLASSIC)
		return symb + (isOpen(bv) ? s : getNewLabel(s));
	else
		return symb + params_.branch + ": " + getNewLabel(s);
}


ColorCode InsetBranch::backgroundColor(PainterInfo const & pi) const
{
	if (params_.branch.empty())
		return Inset::backgroundColor(pi);
	// FIXME UNICODE
	ColorCode c = lcolor.getFromLyXName(to_utf8(params_.branch));
	if (c == Color_none)
		c = Color_error;
	return c;
}


void InsetBranch::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {
	case LFUN_INSET_MODIFY: {
		InsetBranchParams params;
		InsetBranch::string2params(to_utf8(cmd.argument()), params);

		cur.recordUndoInset(this);
		params_.branch = params.branch;
		// what we really want here is a TOC update, but that means
		// a full buffer update
		cur.forceBufferUpdate();
		break;
	}
	case LFUN_BRANCH_ACTIVATE:
	case LFUN_BRANCH_DEACTIVATE:
	case LFUN_BRANCH_MASTER_ACTIVATE:
	case LFUN_BRANCH_MASTER_DEACTIVATE: {
		bool const master = (cmd.action() == LFUN_BRANCH_MASTER_ACTIVATE
				     || cmd.action() == LFUN_BRANCH_MASTER_DEACTIVATE);
		Buffer * buf = master ? const_cast<Buffer *>(buffer().masterBuffer())
				      : &buffer();

		Branch * our_branch = buf->params().branchlist().find(params_.branch);
		if (!our_branch)
			break;

		bool const activate = (cmd.action() == LFUN_BRANCH_ACTIVATE
				       || cmd.action() == LFUN_BRANCH_MASTER_ACTIVATE);
		if (our_branch->isSelected() != activate) {
			// FIXME If the branch is in the master document, we cannot
			// call recordUndo..., because the master may be hidden, and
			// the code presently assumes that hidden documents can never
			// be dirty. See GuiView::closeBufferAll(), for example.
			// An option would be to check if the master is hidden.
			// If it is, unhide.
			if (!master)
				buffer().undo().recordUndoBufferParams(cur);
			else
				// at least issue a warning for now (ugly, but better than dataloss).
				frontend::Alert::warning(_("Branch state changes in master document"),
				    lyx::support::bformat(_("The state of the branch '%1$s' "
					"was changed in the master file. "
					"Please make sure to save the master."), params_.branch), true);
			our_branch->setSelected(activate);
			// cur.forceBufferUpdate() is not enough
			buf->updateBuffer();
		}
		break;
	}
	case LFUN_BRANCH_ADD:
		lyx::dispatch(FuncRequest(LFUN_BRANCH_ADD, params_.branch));
		break;
	case LFUN_INSET_TOGGLE:
		if (cmd.argument() == "assign")
			setStatus(cur, isBranchSelected() ? Open : Collapsed);
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
	bool const known_branch =
		buffer().params().branchlist().find(params_.branch);

	switch (cmd.action()) {
	case LFUN_INSET_MODIFY:
		flag.setEnabled(true);
		break;

	case LFUN_BRANCH_ACTIVATE:
		flag.setEnabled(known_branch && !isBranchSelected(true));
		break;

	case LFUN_BRANCH_ADD:
		flag.setEnabled(!known_branch);
		break;

	case LFUN_BRANCH_DEACTIVATE:
		flag.setEnabled(isBranchSelected(true));
		break;

	case LFUN_BRANCH_MASTER_ACTIVATE:
		flag.setEnabled(buffer().parent()
				&& buffer().masterBuffer()->params().branchlist().find(params_.branch)
				&& !isBranchSelected());
		break;

	case LFUN_BRANCH_MASTER_DEACTIVATE:
		flag.setEnabled(buffer().parent() && isBranchSelected());
		break;

	case LFUN_INSET_TOGGLE:
		if (cmd.argument() == "assign")
			flag.setEnabled(true);
		else
			return InsetCollapsable::getStatus(cur, cmd, flag);	
		break;

	default:
		return InsetCollapsable::getStatus(cur, cmd, flag);
	}
	return true;
}


bool InsetBranch::isBranchSelected(bool const child) const
{
	Buffer const & realbuffer = child ? buffer() : *buffer().masterBuffer();
	BranchList const & branchlist = realbuffer.params().branchlist();
	Branch const * ourBranch = branchlist.find(params_.branch);

	if (!ourBranch) {
		// this branch is defined in child only
		ourBranch = buffer().params().branchlist().find(params_.branch);
		if (!ourBranch)
			return false;
	}
	return ourBranch->isSelected();
}


void InsetBranch::latex(otexstream & os, OutputParams const & runparams) const
{
	if (isBranchSelected())
		InsetText::latex(os, runparams);
}


int InsetBranch::plaintext(odocstringstream & os,
			   OutputParams const & runparams, size_t max_length) const
{
	if (!isBranchSelected())
		return 0;

	int len = InsetText::plaintext(os, runparams, max_length);
	return len;
}


int InsetBranch::docbook(odocstream & os,
			 OutputParams const & runparams) const
{
	return isBranchSelected() ?  InsetText::docbook(os, runparams) : 0;
}


docstring InsetBranch::xhtml(XHTMLStream & xs, OutputParams const & rp) const
{
	if (isBranchSelected()) {
		OutputParams newrp = rp;
		newrp.par_begin = 0;
		newrp.par_end = text().paragraphs().size();
		xhtmlParagraphs(text(), buffer(), xs, newrp);
	}
	return docstring();
}


void InsetBranch::toString(odocstream & os) const
{
	if (isBranchSelected())
		InsetCollapsable::toString(os);
}


void InsetBranch::forOutliner(docstring & os, size_t maxlen) const
{
	if (isBranchSelected())
		InsetCollapsable::forOutliner(os, maxlen);
}


void InsetBranch::validate(LaTeXFeatures & features) const
{
	if (isBranchSelected())
		InsetCollapsable::validate(features);
}


string InsetBranch::contextMenuName() const
{
	return "context-branch";
}


bool InsetBranch::isMacroScope() const 
{
	// Its own scope if not selected by buffer
	return !isBranchSelected();
}


string InsetBranch::params2string(InsetBranchParams const & params)
{
	ostringstream data;
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
	params.read(lex);
}


void InsetBranch::addToToc(DocIterator const & cpit, bool output_active) const
{
	DocIterator pit = cpit;
	pit.push_back(CursorSlice(const_cast<InsetBranch &>(*this)));

	Toc & toc = buffer().tocBackend().toc("branch");
	docstring str = params_.branch + ": ";
	text().forOutliner(str, TOC_ENTRY_LENGTH);
	toc.push_back(TocItem(pit, 0, str, output_active, toolTipText(docstring(), 3, 60)));
	// Proceed with the rest of the inset.
	bool const doing_output = output_active && isBranchSelected();
	InsetCollapsable::addToToc(cpit, doing_output);
}


void InsetBranchParams::write(ostream & os) const
{
	os << to_utf8(branch);
}


void InsetBranchParams::read(Lexer & lex)
{
	lex.eatLine();
	branch = lex.getDocString();
}

} // namespace lyx
