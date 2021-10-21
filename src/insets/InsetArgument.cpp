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
#include "InsetLayout.h"
#include "InsetList.h"
#include "Language.h"
#include "Layout.h"
#include "Lexer.h"
#include "ParIterator.h"
#include "TexRow.h"
#include "texstream.h"
#include "TocBackend.h"
#include "xml.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

using namespace std;

namespace lyx {


InsetArgument::InsetArgument(Buffer * buf, string const & name)
    : InsetCollapsible(buf), name_(name), labelstring_(docstring()),
      font_(inherit_font), labelfont_(inherit_font), decoration_(string()),
      pass_thru_context_(false), pass_thru_local_(false), pass_thru_(false),
      free_spacing_(false), pass_thru_chars_(docstring()), is_toc_caption_(false),
      newline_cmd_(string())
{}


void InsetArgument::write(ostream & os) const
{
	os << "Argument " << name_ << "\n";
	InsetCollapsible::write(os);
}


void InsetArgument::read(Lexer & lex)
{
	lex >> name_;
	InsetCollapsible::read(lex);
}


void InsetArgument::init(Paragraph const & par)
{
	Inset const & ininset = par.inInset();
	bool const insetlayout = !par.layout().hasArgs();
	Layout::LaTeXArgMap const args = insetlayout ?
		ininset.getLayout().args() : par.layout().args();
	pass_thru_context_ = insetlayout ?
		ininset.getLayout().isPassThru() : par.layout().pass_thru;
	// Record PassThru status in order to act on changes.
	bool const former_pass_thru = pass_thru_;

	// Handle pre 2.1 ArgInsets (lyx2lyx cannot classify them)
	// "999" is the conventional name given to those by lyx2lyx
	if (name_ == "999") {
		int const req = insetlayout ? ininset.getLayout().requiredArgs()
				      : par.layout().requiredArgs();
		int const opts = insetlayout ? ininset.getLayout().optArgs()
				      : par.layout().optArgs();
		int nr = 0;
		int ours = 0;
		for (InsetList::Element const & elt : par.insetList()) {
			if (elt.inset->lyxCode() == ARG_CODE) {
				++nr;
				if (elt.inset == this)
					ours = nr;
			}
		}
		bool done = false;
		int realopts = 0;
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
	caption_of_toc_ = string();
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
		newline_cmd_ = (*lait).second.newlinecmd;
		free_spacing_ = (*lait).second.free_spacing;
		docbooktag_ = (*lait).second.docbooktag;
		docbooktagtype_ = (*lait).second.docbooktagtype;
		docbookattr_ = (*lait).second.docbookattr;
		docbookargumentbeforemaintag_ = (*lait).second.docbookargumentbeforemaintag;
		docbookargumentaftermaintag_ = (*lait).second.docbookargumentaftermaintag;
		pass_thru_local_ = false;
		if (lait->second.is_toc_caption) {
			is_toc_caption_ = true;
			// empty if AddToToc is not set
			caption_of_toc_ = insetlayout
				? ininset.getLayout().tocType()
				: par.layout().tocType();
		}

		switch ((*lait).second.passthru) {
			case PT_INHERITED:
				pass_thru_ = pass_thru_context_;
				break;
			case PT_TRUE:
				pass_thru_ = true;
				pass_thru_local_ = true;
				break;
			case PT_FALSE:
				pass_thru_ = false;
				break;
		}
	} else {
		labelstring_ = _("Unknown Argument");
		tooltip_ = _("Argument not known in this Layout. Will be suppressed in the output.");
	}

	if (former_pass_thru != pass_thru_) {
		// PassThru status changed. We might need to update
		// the language of the contents
		// Language const * l  = insetlayout
		// 	? it.inset().buffer().language()
		// 	: it.buffer()->language();
		Language const * l  = ininset.buffer().language();
		fixParagraphLanguage(l);
	}

	setButtonLabel();
}

void InsetArgument::updateBuffer(ParIterator const & it, UpdateType utype, bool const deleted)
{
	init(it.paragraph());
	InsetCollapsible::updateBuffer(it, utype, deleted);
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
	case LFUN_SELF_INSERT:
		// With (only) inherited pass_thru, call Text::dispatch()
		// directly to avoid call for fixParagraphsFont() and/or
		// forcing to latex_language in InsetText::dispatch(),
		// since this does not play nicely with inherited pass_thru
		// (see #8471).
		if (pass_thru_ && !pass_thru_local_) {
			text().dispatch(cur, cmd);
			// For the paste operations, check if we have
			// non-latex_language, and if so, fix.
			if (cmd.action() != LFUN_SELF_INSERT)
				fixParagraphLanguage(buffer().params().language);
		}
		else
			InsetCollapsible::doDispatch(cur, cmd);
		break;

	default:
		InsetCollapsible::doDispatch(cur, cmd);
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
				for (auto const & table : cur.paragraph().insetList())
					if (InsetArgument const * ins = table.inset->asInsetArgument())
						if (ins->name() == type) {
							// we have this already
							flag.setEnabled(false);
							return true;
						}
			} else
				flag.setEnabled(false);
			return true;
		}
		return InsetCollapsible::getStatus(cur, cmd, flag);
	}

	default:
		return InsetCollapsible::getStatus(cur, cmd, flag);
	}
}


string InsetArgument::contextMenuName() const
{
	if (decoration() == InsetDecoration::CONGLOMERATE)
		return "context-argument-conglomerate";
	else
		return "context-argument";
}


FontInfo InsetArgument::getFont() const
{
	if (font_ != inherit_font)
		return font_;
	return InsetCollapsible::getFont();
}


FontInfo InsetArgument::getLabelfont() const
{
	if (labelfont_ != inherit_font)
		return labelfont_;
	return InsetCollapsible::getLabelfont();
}


ColorCode InsetArgument::labelColor() const {
	if (labelfont_.color() != Color_inherit)
		return labelfont_.color();
	return InsetCollapsible::labelColor();
}


InsetDecoration InsetArgument::decoration() const
{
	InsetDecoration dec = getLayout().decoration();
	if (!decoration_.empty())
		dec = translateDecoration(decoration_);
	return dec == InsetDecoration::DEFAULT ? InsetDecoration::CLASSIC : dec;
}


void InsetArgument::docbook(XMLStream & xs, OutputParams const & rp) const {
	// Ignore arguments that have already been output or are planned to be output elsewhere.
	if (rp.docbook_prepended_arguments.find(this) != rp.docbook_prepended_arguments.end())
		return;
	if (rp.docbook_appended_arguments.find(this) != rp.docbook_appended_arguments.end())
		return;

	if (docbooktag_ != from_ascii("NONE") && docbooktag_ != from_ascii("IGNORE")) {
		// TODO: implement docbooktagtype_.
		xs << xml::StartTag(docbooktag_, docbookattr_);
		InsetText::docbook(xs, rp);
		xs << xml::EndTag(docbooktag_);
		xs << xml::CR();
	}
}


void InsetArgument::latexArgument(otexstream & os,
		OutputParams const & runparams_in, docstring const & ldelim,
		docstring const & rdelim, docstring const & presetarg) const
{
	otexstringstream ots;
	OutputParams runparams = runparams_in;
	if (!pass_thru_chars_.empty())
		runparams.pass_thru_chars += pass_thru_chars_;
	if (!newline_cmd_.empty())
		runparams.newlinecmd = newline_cmd_;
	runparams.pass_thru = isPassThru();
	InsetText::latex(ots, runparams);
	TexString ts = ots.release();
	bool const add_braces = !ldelim.empty() && ldelim != "{"
			&& support::contains(ts.str, rdelim);
	os << ldelim;
	if (add_braces)
		os << '{';
	os << presetarg;
	if (!presetarg.empty() && !ts.str.empty())
		os << ", ";
	os << move(ts);
	if (add_braces)
		os << '}';
	os << rdelim;
}


void InsetArgument::addToToc(DocIterator const & dit, bool output_active,
                             UpdateType utype, TocBackend & backend) const
{
	if (!caption_of_toc_.empty()) {
		docstring str;
		text().forOutliner(str, TOC_ENTRY_LENGTH);
		backend.builder(caption_of_toc_).argumentItem(str);
	}
	// Proceed with the rest of the inset.
	InsetText::addToToc(dit, output_active, utype, backend);
}


void InsetArgument::fixParagraphLanguage(Language const * l)
{
	Font font(inherit_font, l);
	if (pass_thru_)
		font.setLanguage(latex_language);
	paragraphs().front().resetFonts(font);
}


} // namespace lyx
