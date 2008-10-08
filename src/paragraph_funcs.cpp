/**
 * \file paragraph_funcs.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "paragraph_funcs.h"

#include "BufferParams.h"
#include "Changes.h"
#include "InsetList.h"
#include "Layout.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "Text.h"
#include "TextClass.h"

#include "support/debug.h"

#include "support/lassert.h"
#include <boost/next_prior.hpp>

using namespace std;

namespace lyx {

static bool moveItem(Paragraph & fromPar, pos_type fromPos,
	Paragraph & toPar, pos_type toPos, BufferParams const & params)
{
	// Note: moveItem() does not honour change tracking!
	// Therefore, it should only be used for breaking and merging paragraphs

	char_type const tmpChar = fromPar.getChar(fromPos);
	Font const tmpFont = fromPar.getFontSettings(params, fromPos);
	Change const tmpChange = fromPar.lookupChange(fromPos);

	if (fromPar.isInset(fromPos)) {
		Inset * tmpInset = 0;
		if (fromPar.getInset(fromPos)) {
			// the inset is not in the paragraph any more
			tmpInset = fromPar.releaseInset(fromPos);
		}
		return toPar.insertInset(toPos, tmpInset, tmpFont, tmpChange);
	} else {
		fromPar.eraseChar(fromPos, false);
		toPar.insertChar(toPos, tmpChar, tmpFont, tmpChange);
	}

	return true;
}


void breakParagraph(BufferParams const & bparams,
		    ParagraphList & pars, pit_type par_offset, pos_type pos, 
		    bool keep_layout)
{
	// create a new paragraph, and insert into the list
	ParagraphList::iterator tmp =
		pars.insert(boost::next(pars.begin(), par_offset + 1),
			    Paragraph());

	Paragraph & par = pars[par_offset];

	// remember to set the inset_owner
	tmp->setInsetOwner(&par.inInset());
	// without doing that we get a crash when typing <Return> at the
	// end of a paragraph
	tmp->setPlainOrDefaultLayout(bparams.documentClass());

	// layout stays the same with latex-environments
	if (keep_layout) {
		tmp->setLayout(par.layout());
		tmp->setLabelWidthString(par.params().labelWidthString());
		tmp->params().depth(par.params().depth());
	} else if (par.params().depth() > 0) {
		Paragraph const & hook = pars[outerHook(par_offset, pars)];
		tmp->setLayout(hook.layout());
		// not sure the line below is useful
		tmp->setLabelWidthString(par.params().labelWidthString());
		tmp->params().depth(hook.params().depth());
	}

	bool const isempty = (par.allowEmpty() && par.empty());

	if (!isempty && (par.size() > pos || par.empty())) {
		tmp->setLayout(par.layout());
		tmp->params().align(par.params().align());
		tmp->setLabelWidthString(par.params().labelWidthString());

		tmp->params().depth(par.params().depth());
		tmp->params().noindent(par.params().noindent());

		// move everything behind the break position
		// to the new paragraph

		/* Note: if !keepempty, empty() == true, then we reach
		 * here with size() == 0. So pos_end becomes - 1. This
		 * doesn't cause problems because both loops below
		 * enforce pos <= pos_end and 0 <= pos
		 */
		pos_type pos_end = par.size() - 1;

		for (pos_type i = pos, j = 0; i <= pos_end; ++i) {
			if (moveItem(par, pos, *tmp, j, bparams)) {
				++j;
			}
		}
	}

	// Move over the end-of-par change information
	tmp->setChange(tmp->size(), par.lookupChange(par.size()));
	par.setChange(par.size(), Change(bparams.trackChanges ?
					   Change::INSERTED : Change::UNCHANGED));

	if (pos) {
		// Make sure that we keep the language when
		// breaking paragraph.
		if (tmp->empty()) {
			Font changed = tmp->getFirstFontSettings(bparams);
			Font old = par.getFontSettings(bparams, par.size());
			changed.setLanguage(old.language());
			tmp->setFont(0, changed);
		}

		return;
	}

	if (!isempty) {
		bool const soa = par.params().startOfAppendix();
		par.params().clear();
		// do not lose start of appendix marker (bug 4212)
		par.params().startOfAppendix(soa);
		par.setPlainOrDefaultLayout(bparams.documentClass());
	}

	// layout stays the same with latex-environments
	if (keep_layout) {
		par.setLayout(tmp->layout());
		par.setLabelWidthString(tmp->params().labelWidthString());
		par.params().depth(tmp->params().depth());
	}
}


void breakParagraphConservative(BufferParams const & bparams,
	ParagraphList & pars, pit_type par_offset, pos_type pos)
{
	// create a new paragraph
	Paragraph & tmp = *pars.insert(boost::next(pars.begin(), par_offset + 1),
				       Paragraph());
	Paragraph & par = pars[par_offset];

	tmp.setInsetOwner(&par.inInset());
	tmp.makeSameLayout(par);

	LASSERT(pos <= par.size(), /**/);

	if (pos < par.size()) {
		// move everything behind the break position to the new paragraph
		pos_type pos_end = par.size() - 1;

		for (pos_type i = pos, j = 0; i <= pos_end; ++i) {
			if (moveItem(par, pos, tmp, j, bparams)) {
				++j;
			}
		}
		// Move over the end-of-par change information
		tmp.setChange(tmp.size(), par.lookupChange(par.size()));
		par.setChange(par.size(), Change(bparams.trackChanges ?
					   Change::INSERTED : Change::UNCHANGED));
	}
}


void mergeParagraph(BufferParams const & bparams,
	ParagraphList & pars, pit_type par_offset)
{
	Paragraph & next = pars[par_offset + 1];
	Paragraph & par = pars[par_offset];

	pos_type pos_end = next.size() - 1;
	pos_type pos_insert = par.size();

	// the imaginary end-of-paragraph character (at par.size()) has to be
	// marked as unmodified. Otherwise, its change is adopted by the first
	// character of the next paragraph.
	if (par.lookupChange(par.size()).type != Change::UNCHANGED) {
		LYXERR(Debug::CHANGES,
		   "merging par with inserted/deleted end-of-par character");
		par.setChange(par.size(), Change(Change::UNCHANGED));
	}

	Change change = next.lookupChange(next.size());

	// move the content of the second paragraph to the end of the first one
	for (pos_type i = 0, j = pos_insert; i <= pos_end; ++i) {
		if (moveItem(next, 0, par, j, bparams)) {
			++j;
		}
	}

	// move the change of the end-of-paragraph character
	par.setChange(par.size(), change);

	pars.erase(boost::next(pars.begin(), par_offset + 1));
}


pit_type depthHook(pit_type pit, ParagraphList const & pars, depth_type depth)
{
	pit_type newpit = pit;

	if (newpit != 0)
		--newpit;

	while (newpit != 0 && pars[newpit].getDepth() > depth)
		--newpit;

	if (pars[newpit].getDepth() > depth)
		return pit;

	return newpit;
}


pit_type outerHook(pit_type par_offset, ParagraphList const & pars)
{
	Paragraph const & par = pars[par_offset];

	if (par.getDepth() == 0)
		return pars.size();
	return depthHook(par_offset, pars, depth_type(par.getDepth() - 1));
}


bool isFirstInSequence(pit_type par_offset, ParagraphList const & pars)
{
	Paragraph const & par = pars[par_offset];

	pit_type dhook_offset = depthHook(par_offset, pars, par.getDepth());

	if (dhook_offset == par_offset)
		return true;

	Paragraph const & dhook = pars[dhook_offset];

	return dhook.layout() != par.layout()
		|| dhook.getDepth() != par.getDepth();
}


int getEndLabel(pit_type p, ParagraphList const & pars)
{
	pit_type pit = p;
	depth_type par_depth = pars[p].getDepth();
	while (pit != pit_type(pars.size())) {
		Layout const & layout = pars[pit].layout();
		int const endlabeltype = layout.endlabeltype;

		if (endlabeltype != END_LABEL_NO_LABEL) {
			if (p + 1 == pit_type(pars.size()))
				return endlabeltype;

			depth_type const next_depth =
				pars[p + 1].getDepth();
			if (par_depth > next_depth ||
			    (par_depth == next_depth && layout != pars[p + 1].layout()))
				return endlabeltype;
			break;
		}
		if (par_depth == 0)
			break;
		pit = outerHook(pit, pars);
		if (pit != pit_type(pars.size()))
			par_depth = pars[pit].getDepth();
	}
	return END_LABEL_NO_LABEL;
}


Font const outerFont(pit_type par_offset, ParagraphList const & pars)
{
	depth_type par_depth = pars[par_offset].getDepth();
	FontInfo tmpfont = inherit_font;

	// Resolve against environment font information
	while (par_offset != pit_type(pars.size())
	       && par_depth
	       && !tmpfont.resolved()) {
		par_offset = outerHook(par_offset, pars);
		if (par_offset != pit_type(pars.size())) {
			tmpfont.realize(pars[par_offset].layout().font);
			par_depth = pars[par_offset].getDepth();
		}
	}

	return Font(tmpfont);
}


void acceptChanges(ParagraphList & pars, BufferParams const & bparams)
{
	pit_type pars_size = static_cast<pit_type>(pars.size());

	// first, accept changes within each individual paragraph
	// (do not consider end-of-par)
	for (pit_type pit = 0; pit < pars_size; ++pit) {
		if (!pars[pit].empty())   // prevent assertion failure
			pars[pit].acceptChanges(bparams, 0, pars[pit].size());
	}

	// next, accept imaginary end-of-par characters
	for (pit_type pit = 0; pit < pars_size; ++pit) {
		pos_type pos = pars[pit].size();

		if (pars[pit].isInserted(pos)) {
			pars[pit].setChange(pos, Change(Change::UNCHANGED));
		} else if (pars[pit].isDeleted(pos)) {
			if (pit == pars_size - 1) {
				// we cannot remove a par break at the end of the last
				// paragraph; instead, we mark it unchanged
				pars[pit].setChange(pos, Change(Change::UNCHANGED));
			} else {
				mergeParagraph(bparams, pars, pit);
				--pit;
				--pars_size;
			}
		}
	}
}


} // namespace lyx
