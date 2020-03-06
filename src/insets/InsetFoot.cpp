/**
 * \file InsetFoot.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetFoot.h"
#include "InsetBox.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Counters.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Layout.h"
#include "OutputParams.h"
#include "ParIterator.h"
#include "TextClass.h"
#include "TocBackend.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

using namespace std;

namespace lyx {

InsetFoot::InsetFoot(Buffer * buf)
	: InsetFootlike(buf), intitle_(false), infloattable_(false)
{}


docstring InsetFoot::layoutName() const
{
	if (intitle_)
		return from_ascii("Foot:InTitle");
	else if (infloattable_)
		return from_ascii("Foot:InFloatTable");
	return from_ascii("Foot");
}


void InsetFoot::updateBuffer(ParIterator const & it, UpdateType utype, bool const deleted)
{
	BufferParams const & bp = buffer().masterBuffer()->params();
	Counters & cnts = bp.documentClass().counters();
	if (utype == OutputUpdate) {
		// the footnote counter is local to this inset
		cnts.saveLastCounter();
	}

	intitle_ = false;
	infloattable_ = false;
	bool intable = false;
	if (it.innerInsetOfType(TABULAR_CODE) != 0)
		intable = true;
	if (it.innerInsetOfType(FLOAT_CODE) != 0)
		infloattable_ = intable;
	// If we are in a table in a float, but the table is also in a minipage,
	// we do not use tablefootnote, since minipages provide their own footnotes.
	if (intable && infloattable_ && it.innerInsetOfType(BOX_CODE) != 0) {
		InsetBoxParams const & boxp =
				static_cast<InsetBox*>(it.innerInsetOfType(BOX_CODE))->params();
		if (boxp.inner_box && !boxp.use_parbox && !boxp.use_makebox)
			infloattable_ = false;
	}
	for (size_type sl = 0 ; sl < it.depth() ; ++sl) {
		if (it[sl].text() && it[sl].paragraph().layout().intitle) {
			intitle_ = true;
			break;
		}
	}

	Language const * lang = it.paragraph().getParLanguage(bp);
	InsetLayout const & il = getLayout();
	docstring const & count = il.counter();
	custom_label_ = translateIfPossible(il.labelstring());

	Paragraph const & par = it.paragraph();
	if (!par.isDeleted(it.pos())) {
		if (cnts.hasCounter(count))
			cnts.step(count, utype);
		custom_label_ += ' ' + cnts.theCounter(count, lang->code());
	} else
		custom_label_ += ' ' + from_ascii("#");
	setLabel(custom_label_);

	InsetCollapsible::updateBuffer(it, utype, deleted);
	if (utype == OutputUpdate)
		cnts.restoreLastCounter();
}


docstring InsetFoot::toolTip(BufferView const & bv, int x, int y) const
{
	if (isOpen(bv))
		// this will give us something useful if there is no button
		return InsetCollapsible::toolTip(bv, x, y);
	return toolTipText(custom_label_+ ": ");
}


int InsetFoot::plaintext(odocstringstream & os,
        OutputParams const & runparams, size_t max_length) const
{
	os << '[' << buffer().B_("footnote") << ":\n";
	InsetText::plaintext(os, runparams, max_length);
	os << "\n]";

	return PLAINTEXT_NEWLINE + 1; // one char on a separate line
}


int InsetFoot::docbook(odocstream & os, OutputParams const & runparams) const
{
	os << "<footnote>";
	int const i = InsetText::docbook(os, runparams);
	os << "</footnote>";

	return i;
}


void InsetFoot::validate(LaTeXFeatures & features) const
{
	// Use footnote package to provide footnotes in tables
	// unless an alternative approach is built in the class.
	if (!features.saveNoteEnv().empty()
	    && !features.isProvided("footnote-alternative")) {
		features.require("footnote");
		features.addPreambleSnippet(
			from_ascii("\\makesavenoteenv{"
				   + features.saveNoteEnv()
				   + "}\n"));
	}

	InsetText::validate(features);
}

} // namespace lyx
