/**
 * \file paragraph_funcs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "paragraph_funcs.h"
#include "paragraph_pimpl.h"
#include "buffer.h"
#include "ParagraphParameters.h"
#include "lyxtextclasslist.h"
#include "debug.h"
#include "language.h"
#include "encoding.h"
#include "lyxrc.h"
#include "support/lstrings.h"
#include "insets/insetoptarg.h"

extern string bibitemWidest(Buffer const *);

using lyx::pos_type;
//using lyx::layout_type;
using std::endl;
using std::ostream;

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

	if (bparams.tracking_changes)
		tmp->trackChanges();

	// this is an idea for a more userfriendly layout handling, I will
	// see what the users say

	// layout stays the same with latex-environments
	if (flag) {
		tmp->layout(par->layout());
		tmp->setLabelWidthString(par->params().labelWidthString());
	}

	bool const isempty = (par->layout()->keepempty && par->empty());

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

#ifdef WITH_WARNINGS
#warning this seems wrong
#endif
		/* FIXME: if !keepempty, empty() == true, then we reach
		 * here with size() == 0. So pos_end becomes - 1. Why
		 * doesn't this cause problems ???
		 */
		pos_type pos_end = par->size() - 1;
		pos_type i = pos;
		pos_type j = pos;

		for (; i <= pos_end; ++i) {
			Change::Type change(par->lookupChange(i));
			par->cutIntoMinibuffer(bparams, i);
			if (tmp->insertFromMinibuffer(j - pos)) {
				tmp->pimpl_->setChange(j - pos, change);
				++j;
			}
		}
		for (i = pos_end; i >= pos; --i) {
			par->pimpl_->eraseIntern(i);
		}
	}

	if (pos)
		return;

	tmp->params().lineTop(par->params().lineTop());
	tmp->params().pagebreakTop(par->params().pagebreakTop());
	tmp->params().spaceTop(par->params().spaceTop());
	par->params().clear();

	par->layout(bparams.getLyXTextClass().defaultLayout());

	// layout stays the same with latex-environments
	if (flag) {
		par->layout(tmp->layout());
		par->setLabelWidthString(tmp->params().labelWidthString());
		par->params().depth(tmp->params().depth());
	}

	// subtle, but needed to get empty pars working right
	if (bparams.tracking_changes) {
		if (!par->size()) {
			par->cleanChanges();
		} else if (!tmp->size()) {
			tmp->cleanChanges();
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


void mergeParagraph(BufferParams const & bparams, Paragraph * par)
{
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


#if 0
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


ParagraphList::iterator
TeXDeeper(Buffer const * buf,
	  BufferParams const & bparams,
	  ParagraphList::iterator pit,
	  ostream & os, TexRow & texrow)
{
	lyxerr[Debug::LATEX] << "TeXDeeper...     " << &*pit << endl;
	ParagraphList::iterator par = pit;

	while (par != buf->paragraphs.end()&& par->params().depth() == pit->params().depth()) {
		if (par->layout()->isEnvironment()) {
			par = TeXEnvironment(buf, bparams, par,
						  os, texrow);
		} else {
			par = TeXOnePar(buf, bparams, par,
					     os, texrow, false);
		}
	}
	lyxerr[Debug::LATEX] << "TeXDeeper...done " << &*par << endl;

	return par;
}


ParagraphList::iterator
TeXEnvironment(Buffer const * buf,
	       BufferParams const & bparams,
	       ParagraphList::iterator pit,
	       ostream & os, TexRow & texrow)
{
	lyxerr[Debug::LATEX] << "TeXEnvironment...     " << &*pit << endl;

	LyXLayout_ptr const & style = pit->layout();

	Language const * language = pit->getParLanguage(bparams);
	Language const * doc_language = bparams.language;
	Language const * previous_language = pit->previous()
		? pit->previous()->getParLanguage(bparams) : doc_language;
	if (language->babel() != previous_language->babel()) {

		if (!lyxrc.language_command_end.empty() &&
		    previous_language->babel() != doc_language->babel()) {
			os << subst(lyxrc.language_command_end, "$$lang",
				    previous_language->babel())
			   << endl;
			texrow.newline();
		}

		if (lyxrc.language_command_end.empty() ||
		    language->babel() != doc_language->babel()) {
			os << subst(lyxrc.language_command_begin, "$$lang",
				    language->babel())
			   << endl;
			texrow.newline();
		}
	}

	bool leftindent_open = false;
	if (!pit->params().leftIndent().zero()) {
		os << "\\begin{LyXParagraphLeftIndent}{" <<
			pit->params().leftIndent().asLatexString() << "}\n";
		texrow.newline();
		leftindent_open = true;
	}

	if (style->isEnvironment()) {
		if (style->latextype == LATEX_LIST_ENVIRONMENT) {
			os << "\\begin{" << style->latexname() << "}{"
			   << pit->params().labelWidthString() << "}\n";
		} else if (style->labeltype == LABEL_BIBLIO) {
			// ale970405
			os << "\\begin{" << style->latexname() << "}{"
			   <<  bibitemWidest(buf)
			   << "}\n";
		} else if (style->latextype == LATEX_ITEM_ENVIRONMENT) {
			os << "\\begin{" << style->latexname() << '}'
			   << style->latexparam() << '\n';
		} else
			os << "\\begin{" << style->latexname() << '}'
			   << style->latexparam() << '\n';
		texrow.newline();
	}
	ParagraphList::iterator par = pit;
	do {
		par = TeXOnePar(buf, bparams, par, os, texrow, false);

		if (par != buf->paragraphs.end()&& par->params().depth() > pit->params().depth()) {
			    if (par->layout()->isParagraph()) {

			    // Thinko!
			    // How to handle this? (Lgb)
			    //&& !suffixIs(os, "\n\n")
				    //) {
				// There should be at least one '\n' already
				// but we need there to be two for Standard
				// paragraphs that are depth-increment'ed to be
				// output correctly.  However, tables can
				// also be paragraphs so don't adjust them.
				// ARRae
				// Thinkee:
				// Will it ever harm to have one '\n' too
				// many? i.e. that we sometimes will have
				// three in a row. (Lgb)
				os << '\n';
				texrow.newline();
			}
			par = TeXDeeper(buf, bparams, par, os, texrow);
		}
	} while (par != buf->paragraphs.end()
		 && par->layout() == pit->layout()
		 && par->params().depth() == pit->params().depth()
		 && par->params().leftIndent() == pit->params().leftIndent());

	if (style->isEnvironment()) {
		os << "\\end{" << style->latexname() << "}\n";
		texrow.newline();
	}

	if (leftindent_open) {
		os << "\\end{LyXParagraphLeftIndent}\n";
		texrow.newline();
	}

	lyxerr[Debug::LATEX] << "TeXEnvironment...done " << &*par << endl;
	return par;  // ale970302
}


namespace {

InsetOptArg * optArgInset(Paragraph const & par)
{
	// Find the entry.
	InsetList::iterator it = par.insetlist.begin();
	InsetList::iterator end = par.insetlist.end();
	for (; it != end; ++it) {
		Inset * ins = it.getInset();
		if (ins->lyxCode() == Inset::OPTARG_CODE) {
			return static_cast<InsetOptArg *>(ins);
		}
	}
	return 0;
}

} // end namespace


ParagraphList::iterator
TeXOnePar(Buffer const * buf,
	  BufferParams const & bparams,
	  ParagraphList::iterator pit,
	  ostream & os, TexRow & texrow,
	  bool moving_arg)
{
	lyxerr[Debug::LATEX] << "TeXOnePar...     " << &*pit << endl;
	Inset const * in = pit->inInset();
	bool further_blank_line = false;
	LyXLayout_ptr style;

	// well we have to check if we are in an inset with unlimited
	// lenght (all in one row) if that is true then we don't allow
	// any special options in the paragraph and also we don't allow
	// any environment other then "Standard" to be valid!
	if ((in == 0) || !in->forceDefaultParagraphs(in)) {
		style = pit->layout();

		if (pit->params().startOfAppendix()) {
			os << "\\appendix\n";
			texrow.newline();
		}

		if (!pit->params().spacing().isDefault()
			&& (!pit->previous() || !pit->previous()->hasSameLayout(&*pit))) {
			os << pit->params().spacing().writeEnvirBegin() << '\n';
			texrow.newline();
		}

		if (style->isCommand()) {
			os << '\n';
			texrow.newline();
		}

		if (pit->params().pagebreakTop()) {
			os << "\\newpage";
			further_blank_line = true;
		}
		if (pit->params().spaceTop().kind() != VSpace::NONE) {
			os << pit->params().spaceTop().asLatexCommand(bparams);
			further_blank_line = true;
		}

		if (pit->params().lineTop()) {
			os << "\\lyxline{\\" << pit->getFont(bparams, 0).latexSize() << '}'
			   << "\\vspace{-1\\parskip}";
			further_blank_line = true;
		}

		if (further_blank_line) {
			os << '\n';
			texrow.newline();
		}
	} else {
		style = bparams.getLyXTextClass().defaultLayout();
	}

	Language const * language = pit->getParLanguage(bparams);
	Language const * doc_language = bparams.language;
	Language const * previous_language = pit->previous()
		? pit->previous()->getParLanguage(bparams) : doc_language;

	if (language->babel() != previous_language->babel()
	    // check if we already put language command in TeXEnvironment()
	    && !(style->isEnvironment()
		 && (!pit->previous() ||
		     (pit->previous()->layout() != pit->layout() &&
		      pit->previous()->getDepth() <= pit->getDepth())
		     || pit->previous()->getDepth() < pit->getDepth())))
	{
		if (!lyxrc.language_command_end.empty() &&
		    previous_language->babel() != doc_language->babel())
		{
			os << subst(lyxrc.language_command_end, "$$lang",
				    previous_language->babel())
			   << endl;
			texrow.newline();
		}

		if (lyxrc.language_command_end.empty() ||
		    language->babel() != doc_language->babel())
		{
			os << subst(lyxrc.language_command_begin, "$$lang",
				    language->babel())
			   << endl;
			texrow.newline();
		}
	}

	if (bparams.inputenc == "auto" &&
	    language->encoding() != previous_language->encoding()) {
		os << "\\inputencoding{"
		   << language->encoding()->LatexName()
		   << "}\n";
		texrow.newline();
	}

	switch (style->latextype) {
	case LATEX_COMMAND:
		os << '\\' << style->latexname();

		// Separate handling of optional argument inset.
		if (style->optionalargs == 1) {
			InsetOptArg * it = optArgInset(*pit);
			if (it)
				it->latexOptional(buf, os, false, false);
		}
		else
			os << style->latexparam();
		break;
	case LATEX_ITEM_ENVIRONMENT:
	case LATEX_LIST_ENVIRONMENT:
		os << "\\item ";
		break;
	case LATEX_BIB_ENVIRONMENT:
		// ignore this, the inset will write itself
		break;
	default:
		break;
	}

	bool need_par = pit->simpleTeXOnePar(buf, bparams, os, texrow, moving_arg);

	// Make sure that \\par is done with the font of the last
	// character if this has another size as the default.
	// This is necessary because LaTeX (and LyX on the screen)
	// calculates the space between the baselines according
	// to this font. (Matthias)
	//
	// Is this really needed ? (Dekel)
	// We do not need to use to change the font for the last paragraph
	// or for a command.
	LyXFont const font =
		(pit->empty()
		 ? pit->getLayoutFont(bparams) : pit->getFont(bparams, pit->size() - 1));

	bool is_command = style->isCommand();

	if (style->resfont.size() != font.size() && pit->next() && !is_command) {
		if (!need_par)
			os << '{';
		os << "\\" << font.latexSize() << " \\par}";
	} else if (need_par) {
		os << "\\par}";
	} else if (is_command)
		os << '}';

	switch (style->latextype) {
	case LATEX_ITEM_ENVIRONMENT:
	case LATEX_LIST_ENVIRONMENT:
		if (pit->next() && (pit->params().depth() < pit->next()->params().depth())) {
			os << '\n';
			texrow.newline();
		}
		break;
	case LATEX_ENVIRONMENT:
		// if its the last paragraph of the current environment
		// skip it otherwise fall through
		if (pit->next()
		    && (pit->next()->layout() != pit->layout()
			|| pit->next()->params().depth() != pit->params().depth()))
			break;
		// fall through possible
	default:
		// we don't need it for the last paragraph!!!
		if (pit->next()) {
			os << '\n';
			texrow.newline();
		}
	}

	if ((in == 0) || !in->forceDefaultParagraphs(in)) {
		further_blank_line = false;
		if (pit->params().lineBottom()) {
			os << "\\lyxline{\\" << font.latexSize() << '}';
			further_blank_line = true;
		}

		if (pit->params().spaceBottom().kind() != VSpace::NONE) {
			os << pit->params().spaceBottom().asLatexCommand(bparams);
			further_blank_line = true;
		}

		if (pit->params().pagebreakBottom()) {
			os << "\\newpage";
			further_blank_line = true;
		}

		if (further_blank_line) {
			os << '\n';
			texrow.newline();
		}

		if (!pit->params().spacing().isDefault()
			&& (!pit->next() || !pit->next()->hasSameLayout(&*pit))) {
			os << pit->params().spacing().writeEnvirEnd() << '\n';
			texrow.newline();
		}
	}

	// we don't need it for the last paragraph!!!
	if (pit->next()) {
		os << '\n';
		texrow.newline();
	} else {
		// Since \selectlanguage write the language to the aux file,
		// we need to reset the language at the end of footnote or
		// float.

		if (language->babel() != doc_language->babel()) {
			if (lyxrc.language_command_end.empty())
				os << subst(lyxrc.language_command_begin,
					    "$$lang",
					    doc_language->babel())
				   << endl;
			else
				os << subst(lyxrc.language_command_end,
					    "$$lang",
					    language->babel())
				   << endl;
			texrow.newline();
		}
	}

	lyxerr[Debug::LATEX] << "TeXOnePar...done " << pit->next() << endl;
	return ++pit;
}
