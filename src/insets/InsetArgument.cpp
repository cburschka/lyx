/**
 * \file InsetArgument.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetArgument.h"

#include "InsetList.h"
#include "Layout.h"
#include "Lexer.h"
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
		int req = insetlayout ? it.inset().getLayout().requiredArgs()
				      : it.paragraph().layout().requiredArgs();
		int opts = insetlayout ? it.inset().getLayout().optArgs()
				      : it.paragraph().layout().optArgs();
		int nr = 0;
		int ours = 0;
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
	Layout::LaTeXArgMap::const_iterator const lait =
			args.find(convert<unsigned int>(name_));
	if (lait != args.end()) {
		labelstring_ = translateIfPossible((*lait).second.labelstring);
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

void InsetArgument::latexArgument(otexstream & os,
		OutputParams const & runparams, docstring const & ldelim,
		docstring const & rdelim) const
{
	TexRow texrow;
	odocstringstream ss;
	otexstream ots(ss, texrow);
	InsetText::latex(ots, runparams);
	docstring str = ss.str();
	if (ldelim != "{" && support::contains(str, rdelim))
		str = '{' + str + '}';
	os << ldelim << str << rdelim;
}


} // namespace lyx
