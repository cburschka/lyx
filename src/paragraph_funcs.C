/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team. 
 *
 * ====================================================== */

#include <config.h>

#include "paragraph_funcs.h"
#include "buffer.h"
#include "ParagraphParameters.h"
#include "lyxtextclasslist.h"
#include "debug.h"

using lyx::pos_type;
//using lyx::layout_type;
using std::endl;


void breakParagraph(BufferParams const & bparams,
		    Paragraph * par,
		    pos_type pos,
		    int flag)
{
	// create a new paragraph
	Paragraph * tmp = new Paragraph(par);
	// without doing that we get a crash when typing <Return> at the
	// end of a paragraph
	tmp->layout(bparams.getLyXTextClass().defaultLayout());
	// remember to set the inset_owner
	tmp->setInsetOwner(par->inInset());
	
	// this is an idea for a more userfriendly layout handling, I will
	// see what the users say
	
	// layout stays the same with latex-environments
	if (flag) {
		tmp->layout(par->layout());
		tmp->setLabelWidthString(par->params().labelWidthString());
	}

	bool isempty = (par->layout()->keepempty && par->empty());
	
	if (!isempty && (par->size() > pos || par->empty() || flag == 2)) {
		tmp->layout(par->layout());
		tmp->params().align(par->params().align());
		tmp->setLabelWidthString(par->params().labelWidthString());
		
		tmp->params().lineBottom(par->params().lineBottom());
		par->params().lineBottom(false);
		tmp->params().pagebreakBottom(par->params().pagebreakBottom());
		par->params().pagebreakBottom(false);
		tmp->params().spaceBottom(par->params().spaceBottom());
		par->params().spaceBottom(VSpace(VSpace::NONE));
		
		tmp->params().depth(par->params().depth());
		tmp->params().noindent(par->params().noindent());
		
		// copy everything behind the break-position
		// to the new paragraph
		pos_type pos_end = par->size() - 1;
		pos_type i = pos;
		pos_type j = pos;
		for (; i <= pos_end; ++i) {
			par->cutIntoMinibuffer(bparams, i);
			if (tmp->insertFromMinibuffer(j - pos))
				++j;
		}
		for (i = pos_end; i >= pos; --i) {
			par->erase(i);
		}
	}
	
	// just an idea of me
	if (!pos) {
		tmp->params().lineTop(par->params().lineTop());
		tmp->params().pagebreakTop(par->params().pagebreakTop());
		tmp->params().spaceTop(par->params().spaceTop());
		tmp->bibkey = par->bibkey;

		par->bibkey = 0;
		par->params().clear();

		par->layout(bparams.getLyXTextClass().defaultLayout());
		
		// layout stays the same with latex-environments
		if (flag) {
			par->layout(tmp->layout());
			par->setLabelWidthString(tmp->params().labelWidthString());
			par->params().depth(tmp->params().depth());
		}
	}
}


void breakParagraphConservative(BufferParams const & bparams,
				Paragraph * par,
				pos_type pos)
{
	// create a new paragraph
	Paragraph * tmp = new Paragraph(par);
	tmp->makeSameLayout(par);

	// When can pos > Last()?
	// I guess pos == Last() is possible.
	if (par->size() > pos) {
		// copy everything behind the break-position to the new
		// paragraph
		pos_type pos_end = par->size() - 1;

		for (pos_type i = pos, j = pos; i <= pos_end; ++i) {
			par->cutIntoMinibuffer(bparams, i);
			if (tmp->insertFromMinibuffer(j - pos))
				++j;
		}
		
		for (pos_type k = pos_end; k >= pos; --k) {
			par->erase(k);
		}
	}
}


#if 0
// Be carefull, this does not make any check at all.
// This method has wrong name, it combined this par with the next par.
// In that sense it is the reverse of break paragraph. (Lgb)
void pasteParagraph(BufferParams const & bparams,
		    Paragraph * par)
{
	// copy the next paragraph to this one
	Paragraph * the_next = par->next();

	// first the DTP-stuff
	par->params().lineBottom(the_next->params().lineBottom());
	par->params().spaceBottom(the_next->params().spaceBottom());
	par->params().pagebreakBottom(the_next->params().pagebreakBottom());

	pos_type pos_end = the_next->size() - 1;
	pos_type pos_insert = par->size();

	// ok, now copy the paragraph
	for (pos_type i = 0, j = 0; i <= pos_end; ++i) {
		the_next->cutIntoMinibuffer(bparams, i);
		if (par->insertFromMinibuffer(pos_insert + j))
			++j;
	}

	// delete the next paragraph
	Paragraph * ppar = the_next->previous();
	Paragraph * npar = the_next->next();
	delete the_next;
	ppar->next(npar);
}


Paragraph * depthHook(Paragraph * par, Paragraph::depth_type depth)
{
	Paragraph * newpar = par;

	do {
		newpar = newpar->previous();
	} while (newpar && newpar->getDepth() > depth);

	if (!newpar) {
		if (par->previous() || par->getDepth())
			lyxerr << "Error (depthHook): "
			       << "no hook." << endl;
		newpar = par;
	}
	return newpar;
}


Paragraph * outerHook(Paragraph * par) 
{
	if (!par->getDepth())
		return 0;
	return depthHook(par, Paragraph::depth_type(par->getDepth() - 1));
}


bool isFirstInSequence(Paragraph * par)
{
	Paragraph const * dhook = depthHook(par, par->getDepth());
	return (dhook == par
		|| dhook->getLayout() != par->getLayout()
		|| dhook->getDepth() != par->getDepth());
}


int getEndLabel(Paragraph * para, BufferParams const & bparams)
{
	Paragraph * par = para;
	while (par) {
		Paragraph::depth_type par_depth = par->getDepth();
		layout_type layout = par->getLayout();
		int const endlabeltype =
			textclasslist.Style(bparams.textclass,
					    layout).endlabeltype;
		if (endlabeltype != END_LABEL_NO_LABEL) {
			if (!para->next())
				return endlabeltype;

			Paragraph::depth_type const next_depth =
				para->next()->getDepth();
			if (par_depth > next_depth ||
			    (par_depth == next_depth
			     && layout != para->next()->getLayout()))
				return endlabeltype;
			break;
		}
		if (par_depth == 0)
			break;
		par = outerHook(par);
	}
	return END_LABEL_NO_LABEL;
}
#endif
