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

#include "buffer.h"
#include "bufferparams.h"

#include "debug.h"
#include "encoding.h"
#include "gettext.h"
#include "iterators.h"
#include "language.h"
#include "lyxtext.h"
#include "outputparams.h"
#include "paragraph_pimpl.h"
#include "sgml.h"
#include "texrow.h"
#include "vspace.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/std_sstream.h"

#include <vector>

using lyx::pos_type;
using lyx::par_type;

using lyx::support::ascii_lowercase;
using lyx::support::atoi;
using lyx::support::bformat;
using lyx::support::compare_ascii_no_case;
using lyx::support::compare_no_case;
using lyx::support::contains;
using lyx::support::split;
using lyx::support::subst;

using std::auto_ptr;
using std::endl;
using std::string;
using std::vector;
using std::istringstream;
using std::ostream;
using std::pair;


namespace {

bool moveItem(Paragraph & from, Paragraph & to,
	BufferParams const & params, pos_type i, pos_type j)
{
	char const tmpchar = from.getChar(i);
	LyXFont tmpfont = from.getFontSettings(params, i);

	if (tmpchar == Paragraph::META_INSET) {
		InsetBase * tmpinset = 0;
		if (from.getInset(i)) {
			// the inset is not in a paragraph anymore
			tmpinset = from.insetlist.release(i);
			from.insetlist.erase(i);
		}

		if (!to.insetAllowed(tmpinset->lyxCode())) {
			delete tmpinset;
			return false;
		}
		if (tmpinset)
			to.insertInset(j, tmpinset, tmpfont);
	} else {
		if (!to.checkInsertChar(tmpfont))
			return false;
		to.insertChar(j, tmpchar, tmpfont);
	}
	return true;
}

}


void breakParagraph(BufferParams const & bparams,
	ParagraphList & pars, par_type par, pos_type pos, int flag)
{
	// create a new paragraph, and insert into the list
	ParagraphList::iterator tmp =
		pars.insert(pars.begin() + par + 1, Paragraph());

	// without doing that we get a crash when typing <Return> at the
	// end of a paragraph
	tmp->layout(bparams.getLyXTextClass().defaultLayout());
	// remember to set the inset_owner
	tmp->setInsetOwner(pars[par].inInset());

	if (bparams.tracking_changes)
		tmp->trackChanges();

	// this is an idea for a more userfriendly layout handling, I will
	// see what the users say

	// layout stays the same with latex-environments
	if (flag) {
		tmp->layout(pars[par].layout());
		tmp->setLabelWidthString(pars[par].params().labelWidthString());
	}

	bool const isempty = (pars[par].allowEmpty() && pars[par].empty());

	if (!isempty && (pars[par].size() > pos || pars[par].empty() || flag == 2)) {
		tmp->layout(pars[par].layout());
		tmp->params().align(pars[par].params().align());
		tmp->setLabelWidthString(pars[par].params().labelWidthString());

		tmp->params().depth(pars[par].params().depth());
		tmp->params().noindent(pars[par].params().noindent());

		// copy everything behind the break-position
		// to the new paragraph

#ifdef WITH_WARNINGS
#warning this seems wrong
#endif
		/* FIXME: if !keepempty, empty() == true, then we reach
		 * here with size() == 0. So pos_end becomes - 1. Why
		 * doesn't this cause problems ???
		 */
		pos_type pos_end = pars[par].size() - 1;
		pos_type i = pos;
		pos_type j = pos;

		for (; i <= pos_end; ++i) {
			Change::Type change = pars[par].lookupChange(i);
			if (moveItem(pars[par], *tmp, bparams, i, j - pos)) {
				tmp->setChange(j - pos, change);
				++j;
			}
		}

		for (i = pos_end; i >= pos; --i)
			pars[par].eraseIntern(i);
	}

	if (pos)
		return;

	pars[par].params().clear();

	pars[par].layout(bparams.getLyXTextClass().defaultLayout());

	// layout stays the same with latex-environments
	if (flag) {
		pars[par].layout(tmp->layout());
		pars[par].setLabelWidthString(tmp->params().labelWidthString());
		pars[par].params().depth(tmp->params().depth());
	}

	// subtle, but needed to get empty pars working right
	if (bparams.tracking_changes) {
		if (!pars[par].size()) {
			pars[par].cleanChanges();
		} else if (!tmp->size()) {
			tmp->cleanChanges();
		}
	}
}


void breakParagraphConservative(BufferParams const & bparams,
	ParagraphList & pars, par_type par, pos_type pos)
{
	// create a new paragraph
	Paragraph & tmp = *pars.insert(pars.begin() + par + 1, Paragraph());
	tmp.makeSameLayout(pars[par]);

	// When can pos > size()?
	// I guess pos == size() is possible.
	if (pars[par].size() > pos) {
		// copy everything behind the break-position to the new
		// paragraph
		pos_type pos_end = pars[par].size() - 1;

		for (pos_type i = pos, j = pos; i <= pos_end; ++i)
			if (moveItem(pars[par], tmp, bparams, i, j - pos))
				++j;

		for (pos_type k = pos_end; k >= pos; --k)
			pars[par].erase(k);
	}
}


void mergeParagraph(BufferParams const & bparams,
	ParagraphList & pars, par_type par)
{
	Paragraph & next = pars[par + 1];

	pos_type pos_end = next.size() - 1;
	pos_type pos_insert = pars[par].size();

	// ok, now copy the paragraph
	for (pos_type i = 0, j = 0; i <= pos_end; ++i)
		if (moveItem(next, pars[par], bparams, i, pos_insert + j))
			++j;

	pars.erase(pars.begin() + par + 1);
}


par_type depthHook(par_type pit,
	ParagraphList const & pars, Paragraph::depth_type depth)
{
	par_type newpit = pit;

	if (newpit != 0)
		--newpit;

	while (newpit != 0 && pars[newpit].getDepth() > depth)
		--newpit;

	if (pars[newpit].getDepth() > depth)
		return pit;

	return newpit;
}


par_type outerHook(par_type par, ParagraphList const & pars)
{
	if (pars[par].getDepth() == 0)
		return pars.size();
	return depthHook(par, pars, Paragraph::depth_type(pars[par].getDepth() - 1));
}


bool isFirstInSequence(par_type pit, ParagraphList const & pars)
{
	par_type dhook = depthHook(pit, pars, pars[pit].getDepth());
	return dhook == pit
		|| pars[dhook].layout() != pars[pit].layout()
		|| pars[dhook].getDepth() != pars[pit].getDepth();
}


int getEndLabel(par_type p, ParagraphList const & pars)
{
	par_type pit = p;
	Paragraph::depth_type par_depth = pars[p].getDepth();
	while (pit != par_type(pars.size())) {
		LyXLayout_ptr const & layout = pars[pit].layout();
		int const endlabeltype = layout->endlabeltype;

		if (endlabeltype != END_LABEL_NO_LABEL) {
			if (p + 1 == par_type(pars.size()))
				return endlabeltype;

			Paragraph::depth_type const next_depth =
				pars[p + 1].getDepth();
			if (par_depth > next_depth ||
			    (par_depth == next_depth && layout != pars[p + 1].layout()))
				return endlabeltype;
			break;
		}
		if (par_depth == 0)
			break;
		pit = outerHook(pit, pars);
		if (pit != par_type(pars.size()))
			par_depth = pars[pit].getDepth();
	}
	return END_LABEL_NO_LABEL;
}


LyXFont const outerFont(par_type pit, ParagraphList const & pars)
{
	Paragraph::depth_type par_depth = pars[pit].getDepth();
	LyXFont tmpfont(LyXFont::ALL_INHERIT);

	// Resolve against environment font information
	while (pit != par_type(pars.size())
	       && par_depth
	       && !tmpfont.resolved()) {
		pit = outerHook(pit, pars);
		if (pit != par_type(pars.size())) {
			tmpfont.realize(pars[pit].layout()->font);
			par_depth = pars[pit].getDepth();
		}
	}

	return tmpfont;
}


par_type outerPar(Buffer const & buf, InsetBase const * inset)
{
	ParIterator pit = const_cast<Buffer &>(buf).par_iterator_begin();
	ParIterator end = const_cast<Buffer &>(buf).par_iterator_end();
	for ( ; pit != end; ++pit) {
		LyXText * text;
		// the second '=' below is intentional
		for (int i = 0; (text = inset->getText(i)); ++i)
			if (&text->paragraphs() == &pit.plist())
				return pit.outerPar();

		InsetList::iterator ii = pit->insetlist.begin();
		InsetList::iterator iend = pit->insetlist.end();
		for ( ; ii != iend; ++ii)
			if (ii->inset == inset)
				return pit.outerPar();
	}
	lyxerr << "outerPar: should not happen" << endl;
	BOOST_ASSERT(false);
	return buf.paragraphs().size(); // shut up compiler
}


Paragraph const & ownerPar(Buffer const & buf, InsetBase const * inset)
{
	ParConstIterator pit = buf.par_iterator_begin();
	ParConstIterator end = buf.par_iterator_end();
	for ( ; pit != end; ++pit) {
		LyXText * text;
		// the second '=' below is intentional
		for (int i = 0; (text = inset->getText(i)); ++i)
			if (&text->paragraphs() == &pit.plist())
				return *pit;

		InsetList::const_iterator ii = pit->insetlist.begin();
		InsetList::const_iterator iend = pit->insetlist.end();
		for ( ; ii != iend; ++ii)
			if (ii->inset == inset)
				return *pit;
	}
	lyxerr << "ownerPar: should not happen" << endl;
	BOOST_ASSERT(false);
	return buf.paragraphs().front(); // shut up compiler
}


/// return the range of pars [beg, end[ owning the range of y [ystart, yend] 
void getParsInRange(ParagraphList & pars, int ystart, int yend,
	par_type & beg, par_type & end)
{
	BOOST_ASSERT(!pars.empty());
	par_type const endpar = pars.size();
	par_type const begpar = 0;

	for (beg = endpar - 1; beg != begpar && pars[beg].y > ystart; --beg)
		;

	for (end = beg ; end != endpar && pars[end].y <= yend; ++end)
		;
}
