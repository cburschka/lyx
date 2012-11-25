/**
 * \file InsetArgument.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetArgument.h"

#include "Cursor.h"
#include "FuncStatus.h"
#include "FuncRequest.h"
#include "InsetList.h"
#include "Layout.h"
#include "Lexer.h"
#include "OutputParams.h"
#include "ParIterator.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

using namespace std;

namespace lyx {


InsetArgument::InsetArgument(Buffer * buf, string const & name)
    : InsetCollapsable(buf), name_(name), labelstring_(docstring())
{}


void InsetArgument::write(ostream & os) const
{
	os << "Argument " << name_ << "\n";
	InsetCollapsable::write(os);
}

void InsetArgument::read(Lexer & lex)
{
	lex >> name_;
	InsetCollapsable::read(lex);
}

void InsetArgument::updateBuffer(ParIterator const & it, UpdateType utype)
{
	Layout::LaTeXArgMap args;
	bool const insetlayout = &it.inset() && it.paragraph().layout().latexargs().empty();
	if (insetlayout)
		args = it.inset().getLayout().latexargs();
	else
		args = it.paragraph().layout().latexargs();

	// Handle pre 2.1 ArgInsets (lyx2lyx cannot classify them)
	if (name_ == "999") {
		unsigned int const req = insetlayout ? it.inset().getLayout().numRequiredArgs()
				      : it.paragraph().layout().requiredArgs();
		unsigned int const opts = insetlayout ? it.inset().getLayout().numOptArgs()
				      : it.paragraph().layout().optArgs();
		unsigned int nr = 0;
		unsigned int ours = 0;
		InsetList::const_iterator parit = it.paragraph().insetList().begin();
		InsetList::const_iterator parend = it.paragraph().insetList().end();
		for (; parit != parend; ++parit) {
			if (parit->inset->lyxCode() == ARG_CODE) {
				++nr;
				if (parit->inset == this)
					ours = nr;
			}
		}
		bool done = false;
		unsigned int realopts = 0;
		if (nr > req) {
			// We have optional arguments
			realopts = nr - req;
			if (ours <= realopts) {
				name_ = convert<string>(ours);
				done = true;
			}
		}
		if (!done) {
			// This is a mandatory argument. We have to consider
			// non-given optional arguments for the numbering
			int offset = opts - realopts;
			ours += offset;
			name_ = convert<string>(ours);
		}
	}
	Layout::LaTeXArgMap::const_iterator const lait =
			args.find(convert<unsigned int>(name_));
	if (lait != args.end()) {
		docstring label = translateIfPossible((*lait).second.labelstring);
		docstring striplabel;
		support::rsplit(label, striplabel, '|');
		labelstring_ = striplabel.empty() ? label: striplabel;
		tooltip_ = translateIfPossible((*lait).second.tooltip);
	} else {
		labelstring_ = _("Unknown Argument");
		tooltip_ = _("Argument not known in this Layout. Will be supressed in the output.");
	}
	setButtonLabel();
	InsetCollapsable::updateBuffer(it, utype);
}

void InsetArgument::setButtonLabel()
{
	setLabel(labelstring_);
}

docstring InsetArgument::toolTip(BufferView const & bv, int, int) const
{
	if (isOpen(bv))
		return tooltip_;
	return toolTipText(tooltip_ + from_ascii(":\n"));
}

void InsetArgument::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY: {
		string const first_arg = cmd.getArg(0);
		bool const change_type = first_arg == "changetype";
		if (!change_type) {
			// not for us
			// this will not be handled higher up
			cur.undispatched();
			return;
		}
		cur.recordUndoInset(ATOMIC_UNDO, this);
		name_ = cmd.getArg(1);
		cur.forceBufferUpdate();
		break;
	}

	default:
		InsetCollapsable::doDispatch(cur, cmd);
		break;
	}
}


bool InsetArgument::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action()) {

	case LFUN_INSET_MODIFY: {
		string const first_arg = cmd.getArg(0);
		if (first_arg == "changetype") {
			string const type = cmd.getArg(1);
			flag.setOnOff(type == name_);
			if (type == name_) {
				flag.setEnabled(true);
				return true;
			}
			Layout::LaTeXArgMap args;
			bool const insetlayout = &cur.inset() && cur.paragraph().layout().latexargs().empty();
			if (insetlayout)
				args = cur.inset().getLayout().latexargs();
			else
				args = cur.paragraph().layout().latexargs();
			Layout::LaTeXArgMap::const_iterator const lait =
					args.find(convert<unsigned int>(type));
			if (lait != args.end()) {
				flag.setEnabled(true);
				InsetList::const_iterator it = cur.paragraph().insetList().begin();
				InsetList::const_iterator end = cur.paragraph().insetList().end();
				for (; it != end; ++it) {
					if (it->inset->lyxCode() == ARG_CODE) {
						InsetArgument const * ins =
							static_cast<InsetArgument const *>(it->inset);
						if (ins->name() == type) {
							// we have this already
							flag.setEnabled(false);
							return true;
						}
					}
				}
			} else
				flag.setEnabled(false);
			return true;
		}
		return InsetCollapsable::getStatus(cur, cmd, flag);
	}

	default:
		return InsetCollapsable::getStatus(cur, cmd, flag);
	}
}

string InsetArgument::contextMenuName() const
{
	return "context-argument";
}

void InsetArgument::latexArgument(otexstream & os,
		OutputParams const & runparams_in, docstring const & ldelim,
		docstring const & rdelim) const
{
	TexRow texrow;
	odocstringstream ss;
	otexstream ots(ss, texrow);
	OutputParams runparams = runparams_in;
	if (getLayout().isPassThru())
		runparams.pass_thru = true;
	InsetText::latex(ots, runparams);
	docstring str = ss.str();
	if (ldelim != "{" && support::contains(str, rdelim))
		str = '{' + str + '}';
	os << ldelim << str << rdelim;
}


} // namespace lyx
