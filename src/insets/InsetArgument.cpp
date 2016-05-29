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

#include "Buffer.h"
#include "BufferParams.h"
#include "Cursor.h"
#include "FuncStatus.h"
#include "FuncRequest.h"
#include "InsetList.h"
#include "Language.h"
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
    : InsetCollapsable(buf), name_(name), labelstring_(docstring()),
      font_(inherit_font), labelfont_(inherit_font), decoration_(string()),
      pass_thru_(false), pass_thru_chars_(docstring())
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
	Layout::LaTeXArgMap args = it.paragraph().layout().args();
	pass_thru_ = it.paragraph().layout().pass_thru;
	bool const insetlayout = args.empty();
	if (insetlayout) {
		args = it.inset().getLayout().args();
		pass_thru_ = it.inset().getLayout().isPassThru();
	}

	// Handle pre 2.1 ArgInsets (lyx2lyx cannot classify them)
	if (name_ == "999") {
		unsigned int const req = insetlayout ? it.inset().getLayout().requiredArgs()
				      : it.paragraph().layout().requiredArgs();
		unsigned int const opts = insetlayout ? it.inset().getLayout().optArgs()
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
	Layout::LaTeXArgMap::const_iterator const lait = args.find(name_);
	if (lait != args.end()) {
		docstring label = translateIfPossible((*lait).second.labelstring);
		docstring striplabel;
		support::rsplit(label, striplabel, '|');
		labelstring_ = striplabel.empty() ? label: striplabel;
		tooltip_ = translateIfPossible((*lait).second.tooltip);
		font_ = (*lait).second.font;
		labelfont_ = (*lait).second.labelfont;
		decoration_ = (*lait).second.decoration;
		pass_thru_chars_ = (*lait).second.pass_thru_chars;
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
		cur.recordUndoInset(this);
		name_ = cmd.getArg(1);
		cur.forceBufferUpdate();
		break;
	}

	case LFUN_PASTE:
	case LFUN_CLIPBOARD_PASTE:
	case LFUN_SELECTION_PASTE:
	case LFUN_PRIMARY_SELECTION_PASTE:
		// Do not call InsetCollapsable::doDispatch(cur, cmd)
		// with (inherited) pass_thru to avoid call for
		// fixParagraphsFont(), which does not play nicely with
		// inherited pass_thru (see #8471).
		// FIXME: Once we have implemented genuine pass_thru
		// option for InsetArgument (not inherited pass_thru),
		// we should probably directly call
		// InsetCollapsable::doDispatch(cur, cmd) for that
		// case as well
		if (pass_thru_)
			text().dispatch(cur, cmd);
		else
			InsetCollapsable::doDispatch(cur, cmd);
		break;

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
			bool const insetlayout = cur.paragraph().layout().latexargs().empty();
			if (insetlayout)
				args = cur.inset().getLayout().latexargs();
			else
				args = cur.paragraph().layout().latexargs();
			Layout::LaTeXArgMap::const_iterator const lait = args.find(type);
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
	if (decoration() == InsetLayout::CONGLOMERATE)
		return "context-argument-conglomerate";
	else
		return "context-argument";
}


FontInfo InsetArgument::getFont() const
{
	if (font_ != inherit_font)
		return font_;
	return getLayout().font();
}


FontInfo InsetArgument::getLabelfont() const
{
	if (labelfont_ != inherit_font)
		return labelfont_;
	return getLayout().labelfont();
}


InsetLayout::InsetDecoration InsetArgument::decoration() const
{
	InsetLayout::InsetDecoration dec = getLayout().decoration();
	if (!decoration_.empty())
		dec = translateDecoration(decoration_);
	return dec == InsetLayout::DEFAULT ? InsetLayout::CLASSIC : dec;
}


void InsetArgument::latexArgument(otexstream & os,
		OutputParams const & runparams_in, docstring const & ldelim,
		docstring const & rdelim, docstring const & presetarg) const
{
	TexRow texrow;
	odocstringstream ss;
	otexstream ots(ss, texrow);
	OutputParams runparams = runparams_in;
	if (!pass_thru_chars_.empty())
		runparams.pass_thru_chars += pass_thru_chars_;
	InsetText::latex(ots, runparams);
	docstring str = ss.str();
	docstring const sep = str.empty() ? docstring() : from_ascii(", ");
	if (!presetarg.empty())
		str = presetarg + sep + str;
	if (ldelim != "{" && support::contains(str, rdelim))
		str = '{' + str + '}';
	os << ldelim << str << rdelim;
}


} // namespace lyx
