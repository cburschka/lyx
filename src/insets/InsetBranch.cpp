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
		support::bformat(_("master %1$s, child %2$s"),
						 masterstatus, childstatus);

	docstring const masteron = producesOutput() ?
		_("on") : _("off");
	docstring const childon =
		(isBranchSelected(true) != params_.inverted) ?
			_("on") : _("off");
	docstring const onoff = (masteron == childon) ?
		masteron :
		support::bformat(_("master %1$s, child %2$s"),
						 masteron, childon);

	docstring const heading = 
		support::bformat(_("Branch Name: %1$s\nBranch Status: %2$s\nInset Status: %3$s"),
						 params_.branch, status, onoff);

	if (isOpen(bv))
		return heading;
	return toolTipText(heading + from_ascii("\n"));
}


docstring const InsetBranch::buttonLabel(BufferView const &) const
{
	static char_type const tick = 0x2714; // ✔ U+2714 HEAVY CHECK MARK
	static char_type const cross = 0x2716; // ✖ U+2716 HEAVY MULTIPLICATION X

	Buffer const & realbuffer = *buffer().masterBuffer();
	BranchList const & branchlist = realbuffer.params().branchlist();
	bool const inmaster = branchlist.find(params_.branch);
	bool const inchild = buffer().params().branchlist().find(params_.branch);

	bool const master_selected = producesOutput();
	bool const child_selected = isBranchSelected(true) != params_.inverted;

	docstring symb = docstring(1, master_selected ? tick : cross);
	if (inchild && master_selected != child_selected)
		symb += (child_selected ? tick : cross);

	if (decoration() == InsetLayout::MINIMALISTIC)
		return symb + params_.branch;

	docstring s;
	if (inmaster && inchild)
		s = _("Branch: ");
	else if (inchild) // && !inmaster
		s = _("Branch (child): ");
	else if (inmaster) // && !inchild
		s = _("Branch (master): ");
	else // !inmaster && !inchild
		s = _("Branch (undefined): ");
	s += params_.branch;

	return symb + s;
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
		params_.inverted = params.inverted;
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
	case LFUN_BRANCH_INVERT:
		cur.recordUndoInset(this);
		params_.inverted = !params_.inverted;
		// what we really want here is a TOC update, but that means
		// a full buffer update
		cur.forceBufferUpdate();
		break;
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

	case LFUN_BRANCH_INVERT:
		flag.setEnabled(true);
		flag.setOnOff(params_.inverted);
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


bool InsetBranch::producesOutput() const
{
	return isBranchSelected() != params_.inverted;
}


void InsetBranch::latex(otexstream & os, OutputParams const & runparams) const
{
	if (producesOutput())
		InsetText::latex(os, runparams);
}


int InsetBranch::plaintext(odocstringstream & os,
			   OutputParams const & runparams, size_t max_length) const
{
	if (!producesOutput())
		return 0;

	int len = InsetText::plaintext(os, runparams, max_length);
	return len;
}


int InsetBranch::docbook(odocstream & os,
			 OutputParams const & runparams) const
{
	return producesOutput() ?  InsetText::docbook(os, runparams) : 0;
}


docstring InsetBranch::xhtml(XHTMLStream & xs, OutputParams const & rp) const
{
	if (producesOutput()) {
		OutputParams newrp = rp;
		newrp.par_begin = 0;
		newrp.par_end = text().paragraphs().size();
		xhtmlParagraphs(text(), buffer(), xs, newrp);
	}
	return docstring();
}


void InsetBranch::toString(odocstream & os) const
{
	if (producesOutput())
		InsetCollapsable::toString(os);
}


void InsetBranch::forOutliner(docstring & os, size_t const maxlen,
							  bool const shorten) const
{
	if (producesOutput())
		InsetCollapsable::forOutliner(os, maxlen, shorten);
}


void InsetBranch::validate(LaTeXFeatures & features) const
{
	if (producesOutput())
		InsetCollapsable::validate(features);
}


string InsetBranch::contextMenuName() const
{
	return "context-branch";
}


bool InsetBranch::isMacroScope() const 
{
	// Its own scope if not selected by buffer
	return !producesOutput();
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


void InsetBranch::addToToc(DocIterator const & cpit, bool output_active,
						   UpdateType utype) const
{
	DocIterator pit = cpit;
	pit.push_back(CursorSlice(const_cast<InsetBranch &>(*this)));

	docstring str;
	text().forOutliner(str, TOC_ENTRY_LENGTH);
	str = params_.branch + (params_.inverted ? " (-):" : ": ") + str;

	shared_ptr<Toc> toc = buffer().tocBackend().toc("branch");
	toc->push_back(TocItem(pit, 0, str, output_active));

	// Proceed with the rest of the inset.
	bool const doing_output = output_active && producesOutput();
	InsetCollapsable::addToToc(cpit, doing_output, utype);
}


void InsetBranchParams::write(ostream & os) const
{
	os << to_utf8(branch) 
	   << '\n' 
	   << "inverted " 
	   << inverted;
}


void InsetBranchParams::read(Lexer & lex)
{
	lex >> branch;
	lex >> "inverted" >> inverted;
}

} // namespace lyx
