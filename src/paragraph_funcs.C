/**
 * \file paragraph_funcs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "paragraph_funcs.h"

#include "bufferparams.h"
#include "lyxtext.h"
#include "paragraph_pimpl.h"


namespace lyx {

using std::string;


static bool moveItem(Paragraph & fromPar, pos_type fromPos,
	Paragraph & toPar, pos_type toPos, BufferParams const & params)
{
	Paragraph::value_type const tmpChar = fromPar.getChar(fromPos);
	LyXFont const tmpFont = fromPar.getFontSettings(params, fromPos);
	Change const tmpChange = fromPar.lookupChange(fromPos);

	if (tmpChar == Paragraph::META_INSET) {
		InsetBase * tmpInset = 0;
		if (fromPar.getInset(fromPos)) {
			// the inset is not in a paragraph anymore
			tmpInset = fromPar.insetlist.release(fromPos);
			fromPar.insetlist.erase(fromPos);
		}

		if (!toPar.insetAllowed(tmpInset->lyxCode())) {
			delete tmpInset;
			return false;
		}
		if (tmpInset)
			toPar.insertInset(toPos, tmpInset, tmpFont, tmpChange);
	} else {
		toPar.insertChar(toPos, tmpChar, tmpFont, tmpChange);
	}
	return true;
}


void breakParagraph(BufferParams const & bparams,
	ParagraphList & pars, pit_type par_offset, pos_type pos, int flag)
{
	// create a new paragraph, and insert into the list
	ParagraphList::iterator tmp =
		pars.insert(boost::next(pars.begin(), par_offset + 1),
			    Paragraph());

	Paragraph & par = pars[par_offset];

	// we will invalidate the row cache
	par.rows().clear();

	// without doing that we get a crash when typing <Return> at the
	// end of a paragraph
	tmp->layout(bparams.getLyXTextClass().defaultLayout());
	// remember to set the inset_owner
	tmp->setInsetOwner(par.inInset());

	// this is an idea for a more userfriendly layout handling, I will
	// see what the users say

	// layout stays the same with latex-environments
	if (flag) {
		tmp->layout(par.layout());
		tmp->setLabelWidthString(par.params().labelWidthString());
		tmp->params().depth(par.params().depth());
	} else if (par.params().depth() > 0) {
		Paragraph const & hook = pars[outerHook(par_offset, pars)];
		tmp->layout(hook.layout());
		// not sure the line below is useful
		tmp->setLabelWidthString(par.params().labelWidthString());
		tmp->params().depth(hook.params().depth());
	}

	bool const isempty = (par.allowEmpty() && par.empty());

	if (!isempty && (par.size() > pos || par.empty() || flag == 2)) {
		tmp->layout(par.layout());
		tmp->params().align(par.params().align());
		tmp->setLabelWidthString(par.params().labelWidthString());

		tmp->params().depth(par.params().depth());
		tmp->params().noindent(par.params().noindent());

		// copy everything behind the break-position
		// to the new paragraph

		/* Note: if !keepempty, empty() == true, then we reach
		 * here with size() == 0. So pos_end becomes - 1. This
		 * doesn't cause problems because both loops below
		 * enforce pos <= pos_end and 0 <= pos
		 */
		pos_type pos_end = par.size() - 1;

		for (pos_type i = pos, j = pos; i <= pos_end; ++i) {
			if (moveItem(par, i, *tmp, j - pos, bparams)) {
				++j;
			}
		}

		for (pos_type i = pos_end; i >= pos; --i)
			// FIXME: change tracking (MG)
			par.eraseChar(i, false); // erase without change tracking
	}

	if (pos) {
		// Make sure that we keep the language when
		// breaking paragrpah.
		if (tmp->empty()) {
			LyXFont changed = tmp->getFirstFontSettings(bparams);
			LyXFont old = par.getFontSettings(bparams, par.size());
			changed.setLanguage(old.language());
			tmp->setFont(0, changed);
		}

		return;
	}

	if (!isempty) {
		par.params().clear();
		par.layout(bparams.getLyXTextClass().defaultLayout());
	}

	// layout stays the same with latex-environments
	if (flag) {
		par.layout(tmp->layout());
		par.setLabelWidthString(tmp->params().labelWidthString());
		par.params().depth(tmp->params().depth());
	}

	// subtle, but needed to get empty pars working right
	if (bparams.trackChanges) {
		// FIXME: Change tracking (MG)
		// if (!par.size())
		//	set 'par' text to INSERTED in CT mode; clear CT info otherwise
		// else if (!tmp->size())
		//	set 'tmp' text to INSERTED in CT mode; clear CT info otherwise
	}
}


void breakParagraphConservative(BufferParams const & bparams,
	ParagraphList & pars, pit_type par_offset, pos_type pos)
{
	// create a new paragraph
	Paragraph & tmp = *pars.insert(boost::next(pars.begin(), par_offset + 1),
				       Paragraph());
	Paragraph & par = pars[par_offset];

	tmp.makeSameLayout(par);

	// When can pos > size()?
	// I guess pos == size() is possible.
	if (par.size() > pos) {
		// copy everything behind the break-position to the new
		// paragraph
		pos_type pos_end = par.size() - 1;

		for (pos_type i = pos, j = pos; i <= pos_end; ++i) {
			if (moveItem(par, i, tmp, j - pos, bparams)) {
				++j;
			}
		}
		// Move over end-of-par change attr
		// FIXME: change tracking (MG)
		tmp.setChange(tmp.size(), Change(par.lookupChange(par.size()).type));

		// If tracking changes, set all the text that is to be
		// erased to Type::INSERTED.
		for (pos_type k = pos_end; k >= pos; --k) {
			if (bparams.trackChanges)
				// FIXME: Change tracking (MG)
				par.setChange(k, Change(Change::INSERTED));
			// FIXME: change tracking (MG)
			par.eraseChar(k, false);
		}
	}
}


void mergeParagraph(BufferParams const & bparams,
	ParagraphList & pars, pit_type par_offset)
{
	Paragraph & next = pars[par_offset + 1];
	Paragraph & par = pars[par_offset];

	pos_type pos_end = next.size() - 1;
	pos_type pos_insert = par.size();

	// What happens is the following. Later on, moveItem() will copy
	// over characters from the next paragraph to be inserted into this
	// position. Now, if the first char to be so copied is "red" (i.e.,
	// marked deleted) and the paragraph break is marked "blue",
	// insertChar will trigger (eventually, through record(), and see
	// del() and erase() in changes.C) a "hard" character deletion.
	// Which doesn't make sense of course at this pos, but the effect is
	// to shorten the change range to which this para break belongs, by
	// one. It will (should) remain "orphaned", having no CT info to it,
	// and check() in changes.C will assert. Setting the para break
	// forcibly to "black" prevents this scenario. -- MV 13.3.2006
	// FIXME: change tracking (MG)
	par.setChange(par.size(), Change(Change::UNCHANGED));

	Change::Type cr = next.lookupChange(next.size()).type;
	// ok, now copy the paragraph
	for (pos_type i = 0, j = 0; i <= pos_end; ++i) {
		if (moveItem(next, i, par, pos_insert + j, bparams)) {
			++j;
		}
	}
	// Move the change status of "carriage return" over
	// FIXME: change tracking (MG)
	par.setChange(par.size(), Change(cr));

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

	Paragraph const & dhook = pars[dhook_offset];

	return dhook_offset == par_offset
		|| dhook.layout() != par.layout()
		|| dhook.getDepth() != par.getDepth();
}


int getEndLabel(pit_type p, ParagraphList const & pars)
{
	pit_type pit = p;
	depth_type par_depth = pars[p].getDepth();
	while (pit != pit_type(pars.size())) {
		LyXLayout_ptr const & layout = pars[pit].layout();
		int const endlabeltype = layout->endlabeltype;

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


LyXFont const outerFont(pit_type par_offset, ParagraphList const & pars)
{
	depth_type par_depth = pars[par_offset].getDepth();
	LyXFont tmpfont(LyXFont::ALL_INHERIT);

	// Resolve against environment font information
	while (par_offset != pit_type(pars.size())
	       && par_depth
	       && !tmpfont.resolved()) {
		par_offset = outerHook(par_offset, pars);
		if (par_offset != pit_type(pars.size())) {
			tmpfont.realize(pars[par_offset].layout()->font);
			par_depth = pars[par_offset].getDepth();
		}
	}

	return tmpfont;
}


/// return the number of InsetOptArg in a paragraph
int numberOfOptArgs(Paragraph const & par)
{
	int num = 0;

	InsetList::const_iterator it = par.insetlist.begin();
	InsetList::const_iterator end = par.insetlist.end();
	for (; it != end ; ++it) {
		if (it->inset->lyxCode() == InsetBase::OPTARG_CODE)
			++num;
	}
	return num;
}


} // namespace lyx
