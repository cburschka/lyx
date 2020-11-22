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
#include "BufferEncodings.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Lexer.h"
#include "LyX.h"
#include "MetricsInfo.h"
#include "texstream.h"

#include "insets/InsetBox.h"
#include "insets/InsetBranch.h"
#include "insets/InsetCommand.h"
#include "insets/InsetERT.h"
#include "insets/InsetExternal.h"
#include "insets/InsetFloat.h"
#include "insets/InsetGraphics.h"
#include "insets/InsetIndex.h"
#include "insets/InsetLine.h"
#include "insets/InsetListings.h"
#include "insets/InsetNote.h"
#include "insets/InsetPhantom.h"
#include "insets/InsetSpace.h"
#include "insets/InsetTabular.h"
#include "insets/InsetVSpace.h"
#include "insets/InsetWrap.h"

#include "support/debug.h"
#include "support/lstrings.h"

#include "frontends/Application.h"

#include <sstream>

using namespace std;
using namespace lyx::support;


namespace lyx {

// FIXME Would it now be possible to use the InsetCode in
// place of the mailer name and recover that information?
InsetCommand::InsetCommand(Buffer * buf, InsetCommandParams const & p)
	: Inset(buf), p_(p), broken_(false)
{}


// The sole purpose of this copy constructor is to make sure
// that the mouse_hover_ map is not copied and remains empty.
InsetCommand::InsetCommand(InsetCommand const & rhs)
	: Inset(rhs), p_(rhs.p_), broken_(false)
{}


InsetCommand & InsetCommand::operator=(InsetCommand const & rhs)
{
	if (&rhs == this)
		return *this;

	Inset::operator=(rhs);
	p_ = rhs.p_;
	mouse_hover_.clear();
	button_ = RenderButton();
	broken_ = false;

	return *this;
}


InsetCommand::~InsetCommand()
{
	if (p_.code() != NO_CODE)
		hideDialogs(insetName(p_.code()), this);

	map<BufferView const *, bool>::iterator it = mouse_hover_.begin();
	map<BufferView const *, bool>::iterator end = mouse_hover_.end();
	for (; it != end; ++it)
		if (it->second)
			it->first->clearLastInset(this);
}


void InsetCommand::metrics(MetricsInfo & mi, Dimension & dim) const
{
	button_.update(screenLabel(), editable() || clickable(*mi.base.bv, 0, 0),
	               inheritFont(), broken_);
	button_.metrics(mi, dim);
}


bool InsetCommand::setMouseHover(BufferView const * bv, bool mouse_hover)
	const
{
	mouse_hover_[bv] = mouse_hover;
	return true;
}


void InsetCommand::draw(PainterInfo & pi, int x, int y) const
{
	button_.setRenderState(mouse_hover_[pi.base.bv]);
	button_.draw(pi, x, y);
}


void InsetCommand::setParam(string const & name, docstring const & value)
{
	p_[name] = value;
}


docstring const & InsetCommand::getParam(string const & name) const
{
	return p_[name];
}


void InsetCommand::setParams(InsetCommandParams const & p)
{
	p_ = p;
	initView();
}


void InsetCommand::latex(otexstream & os, OutputParams const & runparams_in) const
{
	OutputParams runparams = runparams_in;
	docstring command = getCommand(runparams);
	if (buffer().params().use_minted
	    && prefixIs(command, from_ascii("\\lstlistoflistings")))
		command.erase(1, 3);
	os << command;
}


int InsetCommand::plaintext(odocstringstream & os,
        OutputParams const &, size_t) const
{
	docstring const str = "[" + buffer().B_("LaTeX Command: ")
		+ from_utf8(getCmdName()) + "]";
	os << str;
	return str.size();
}


void InsetCommand::docbook(XMLStream &, OutputParams const &) const
{
	return;
}


void InsetCommand::validate(LaTeXFeatures & features) const
{
	if (params().info().hasParam("literal")
	    && params()["literal"] == "true")
		return;

	ParamInfo::const_iterator it = params().info().begin();
	ParamInfo::const_iterator end = params().info().end();
	for (; it != end; ++it) {
		if (it->handling() == ParamInfo::HANDLING_LATEXIFY) {
			docstring const text = params()[it->name()];
			// Validate the contents (if we LaTeXify, specific
			// macros might require packages)
			for (pos_type i = 0; i < int(text.size()) ; ++i)
				BufferEncodings::validate(text[i], features);
		}
	}
}


void InsetCommand::changeCmdName(string const & new_name)
{
	string const & old_name = getCmdName();
	if (old_name == new_name)
		return;

	if (buffer().masterParams().track_changes) {
		// With change tracking, we insert a new inset and
		// delete the old one
		InsetCommandParams p(p_.code());
		p = p_;
		p.setCmdName(new_name);
		string const data = InsetCommand::params2string(p);
		lyx::dispatch(FuncRequest(LFUN_INSET_INSERT, data));
		lyx::dispatch(FuncRequest(LFUN_CHAR_DELETE_FORWARD));
	} else
		p_.setCmdName(new_name);
}


void InsetCommand::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {
	case LFUN_INSET_MODIFY: {
		if (cmd.getArg(0) == "changetype") {
			cur.recordUndo();
			changeCmdName(cmd.getArg(1));
			cur.forceBufferUpdate();
			initView();
			break;
		}
		InsetCommandParams p(p_.code());
		InsetCommand::string2params(to_utf8(cmd.argument()), p);
		if (p.getCmdName().empty())
			cur.noScreenUpdate();
		else {
			cur.recordUndo();
			if (buffer().masterParams().track_changes) {
				// With change tracking, we insert a new inset and
				// delete the old one
				string const data = InsetCommand::params2string(p);
				lyx::dispatch(FuncRequest(LFUN_INSET_INSERT, data));
				lyx::dispatch(FuncRequest(LFUN_CHAR_DELETE_FORWARD));
			} else
				setParams(p);
		}
		// FIXME We might also want to check here if this one is in the TOC.
		// But I think most of those are labeled.
		if (isLabeled())
			cur.forceBufferUpdate();
		break;
	}

	case LFUN_INSET_DIALOG_UPDATE: {
		string const name = to_utf8(cmd.argument());
		cur.bv().updateDialog(name, params2string(params()));
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
	switch (cmd.action()) {
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

	default:
		return Inset::getStatus(cur, cmd, status);
	}
}


string InsetCommand::contextMenuName() const
{
	return "context-" + insetName(p_.code());
}


bool InsetCommand::showInsetDialog(BufferView * bv) const
{
	if (p_.code() != NO_CODE)
		bv->showDialog(insetName(p_.code()), params2string(p_),
			const_cast<InsetCommand *>(this));
	return true;
}


bool InsetCommand::string2params(string const & data,
	InsetCommandParams & params)
{
	params.clear();
	if (data.empty())
		return false;
	// This happens when inset-insert is called without argument except for the
	// inset type; ex:
	// "inset-insert toc"
	string const name = insetName(params.code());
	if (data == name)
		return true;
	istringstream dstream(data);
	Lexer lex;
	lex.setStream(dstream);
	lex.setContext("InsetCommand::string2params");
	lex >> name.c_str(); // check for name
	lex >> "CommandInset";
	params.read(lex);
	return true;
}


string InsetCommand::params2string(InsetCommandParams const & params)
{
	ostringstream data;
	data << insetName(params.code()) << ' ';
	params.write(data);
	data << "\\end_inset\n";
	return data.str();
}


bool decodeInsetParam(string const & name, string & data,
	Buffer const & buffer)
{
	InsetCode const code = insetCode(name);
	switch (code) {
	case BIBITEM_CODE:
	case BIBTEX_CODE:
	case INDEX_PRINT_CODE:
	case LABEL_CODE:
	case LINE_CODE:
	case NOMENCL_CODE:
	case NOMENCL_PRINT_CODE:
	case REF_CODE:
	case TOC_CODE:
	case HYPERLINK_CODE:
	case COUNTER_CODE: {
		InsetCommandParams p(code);
		data = InsetCommand::params2string(p);
		break;
	}
	case INCLUDE_CODE: {
		// data is the include type: one of "include",
		// "input", "verbatiminput" or "verbatiminput*"
		if (data.empty())
			// default type is requested
			data = "include";
		InsetCommandParams p(INCLUDE_CODE, data);
		data = InsetCommand::params2string(p);
		break;
	}
	case BOX_CODE: {
		// \c data == "Boxed" || "Frameless" etc
		InsetBoxParams p(data);
		data = InsetBox::params2string(p);
		break;
	}
	case BRANCH_CODE: {
		InsetBranchParams p;
		data = InsetBranch::params2string(p);
		break;
	}
	case CITE_CODE: {
		InsetCommandParams p(CITE_CODE);
		data = InsetCommand::params2string(p);
		break;
	}
	case ERT_CODE: {
		data = InsetERT::params2string(InsetCollapsible::Open);
		break;
	}
	case EXTERNAL_CODE: {
		InsetExternalParams p;
		data = InsetExternal::params2string(p, buffer);
		break;
	}
	case FLOAT_CODE:  {
		InsetFloatParams p;
		data = InsetFloat::params2string(p);
		break;
	}
	case INDEX_CODE: {
		InsetIndexParams p;
		data = InsetIndex::params2string(p);
		break;
	}
	case LISTINGS_CODE: {
		InsetListingsParams p;
		data = InsetListings::params2string(p);
		break;
	}
	case GRAPHICS_CODE: {
		InsetGraphicsParams p;
		data = InsetGraphics::params2string(p, buffer);
		break;
	}
	case MATH_SPACE_CODE: {
		InsetSpaceParams p(true);
		data = InsetSpace::params2string(p);
		break;
	}
	case NOTE_CODE: {
		InsetNoteParams p;
		data = InsetNote::params2string(p);
		break;
	}
	case PHANTOM_CODE: {
		InsetPhantomParams p;
		data = InsetPhantom::params2string(p);
		break;
	}
	case SPACE_CODE: {
		InsetSpaceParams p;
		data = InsetSpace::params2string(p);
		break;
	}
	case VSPACE_CODE: {
		VSpace space;
		data = InsetVSpace::params2string(space);
		break;
	}
	case WRAP_CODE: {
		InsetWrapParams p;
		data = InsetWrap::params2string(p);
		break;
	}
	default:
		return false;
	} // end switch(code)
	return true;
}

} // namespace lyx
