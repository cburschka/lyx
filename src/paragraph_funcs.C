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
#include "gettext.h"
#include "language.h"
#include "encoding.h"
#include "lyxrc.h"
#include "lyxlex.h"
#include "factory.h"
#include "Lsstream.h"

#include "support/lstrings.h"

#include "insets/insetoptarg.h"
#include "insets/insetcommandparams.h"
#include "insets/insetbibitem.h"
#include "insets/insetspace.h"
#include "insets/insetspecialchar.h"
#include "insets/insetlatexaccent.h"
#include "insets/insettabular.h"
#include "insets/insethfill.h"
#include "insets/insetnewline.h"

extern string bibitemWidest(Buffer const *);

using namespace lyx::support;

using lyx::pos_type;
//using lyx::layout_type;
using std::endl;
using std::ostream;


void breakParagraph(BufferParams const & bparams,
		    ParagraphList & paragraphs,
		    ParagraphList::iterator par,
		    pos_type pos,
		    int flag)
{
	// create a new paragraph, and insert into the list
	ParagraphList::iterator tmp = paragraphs.insert(boost::next(par),
							Paragraph());

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

	bool const isempty = (par->allowEmpty() && par->empty());

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
				tmp->setChange(j - pos, change);
				++j;
			}
		}
		for (i = pos_end; i >= pos; --i) {
			par->eraseIntern(i);
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
				ParagraphList & paragraphs,
				ParagraphList::iterator par,
				pos_type pos)
{
	// create a new paragraph
	ParagraphList::iterator tmp = paragraphs.insert(boost::next(par),
							Paragraph());
	tmp->makeSameLayout(*par);

	// When can pos > size()?
	// I guess pos == size() is possible.
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


void mergeParagraph(BufferParams const & bparams,
		    ParagraphList & paragraphs,
		    ParagraphList::iterator par)
{
	ParagraphList::iterator the_next = boost::next(par);

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

	paragraphs.erase(the_next);
}


ParagraphList::iterator depthHook(ParagraphList::iterator pit,
				  ParagraphList const & plist,
				  Paragraph::depth_type depth)
{
	ParagraphList::iterator newpit = pit;
	ParagraphList::iterator beg = const_cast<ParagraphList&>(plist).begin();

	if (newpit != beg)
		--newpit;

	while (newpit !=  beg && newpit->getDepth() > depth) {
		--newpit;
	}

	if (newpit->getDepth() > depth)
		return pit;

	return newpit;
}


ParagraphList::iterator outerHook(ParagraphList::iterator pit,
				  ParagraphList const & plist)
{
	if (!pit->getDepth())
		return const_cast<ParagraphList&>(plist).end();
	return depthHook(pit, plist,
			 Paragraph::depth_type(pit->getDepth() - 1));
}


bool isFirstInSequence(ParagraphList::iterator pit,
		       ParagraphList const & plist)
{
	ParagraphList::iterator dhook = depthHook(pit, plist, pit->getDepth());
	return (dhook == pit
		|| dhook->layout() != pit->layout()
		|| dhook->getDepth() != pit->getDepth());
}


int getEndLabel(ParagraphList::iterator p, ParagraphList const & plist)
{
	ParagraphList::iterator pit = p;
	Paragraph::depth_type par_depth = p->getDepth();
	while (pit != const_cast<ParagraphList&>(plist).end()) {
		LyXLayout_ptr const & layout = pit->layout();
		int const endlabeltype = layout->endlabeltype;

		if (endlabeltype != END_LABEL_NO_LABEL) {
			if (boost::next(p) == const_cast<ParagraphList&>(plist).end())
				return endlabeltype;

			Paragraph::depth_type const next_depth = boost::next(p)->getDepth();
			if (par_depth > next_depth ||
			    (par_depth == next_depth &&
			     layout != boost::next(p)->layout()))
				return endlabeltype;
			break;
		}
		if (par_depth == 0)
			break;
		pit = outerHook(pit, plist);
		if (pit != const_cast<ParagraphList&>(plist).end())
			par_depth = pit->getDepth();
	}
	return END_LABEL_NO_LABEL;
}


namespace {

ParagraphList::iterator
TeXEnvironment(Buffer const * buf,
	       ParagraphList const & paragraphs,
	       ParagraphList::iterator pit,
	       ostream & os, TexRow & texrow,
	       LatexRunParams const & runparams);

ParagraphList::iterator
TeXOnePar(Buffer const * buf,
	  ParagraphList const & paragraphs,
	  ParagraphList::iterator pit,
	  ostream & os, TexRow & texrow,
	  LatexRunParams const & runparams,
	  string const & everypar = string());


ParagraphList::iterator
TeXDeeper(Buffer const * buf,
	  ParagraphList const & paragraphs,
	  ParagraphList::iterator pit,
	  ostream & os, TexRow & texrow,
	  LatexRunParams const & runparams)
{
	lyxerr[Debug::LATEX] << "TeXDeeper...     " << &*pit << endl;
	ParagraphList::iterator par = pit;

	while (par != const_cast<ParagraphList&>(paragraphs).end() &&
		     par->params().depth() == pit->params().depth()) {
		if (par->layout()->isEnvironment()) {
			par = TeXEnvironment(buf, paragraphs, par,
					     os, texrow, runparams);
		} else {
			par = TeXOnePar(buf, paragraphs, par,
					     os, texrow, runparams);
		}
	}
	lyxerr[Debug::LATEX] << "TeXDeeper...done " << &*par << endl;

	return par;
}


ParagraphList::iterator
TeXEnvironment(Buffer const * buf,
	       ParagraphList const & paragraphs,
	       ParagraphList::iterator pit,
	       ostream & os, TexRow & texrow,
	       LatexRunParams const & runparams)
{
	lyxerr[Debug::LATEX] << "TeXEnvironment...     " << &*pit << endl;

	BufferParams const & bparams = buf->params;

	LyXLayout_ptr const & style = pit->layout();

	Language const * language = pit->getParLanguage(bparams);
	Language const * doc_language = bparams.language;
	Language const * previous_language =
		(pit != const_cast<ParagraphList&>(paragraphs).begin())
		? boost::prior(pit)->getParLanguage(bparams)
		: doc_language;
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
		par = TeXOnePar(buf, paragraphs, par, os, texrow, runparams);

		if (par != const_cast<ParagraphList&>(paragraphs).end() && par->params().depth() > pit->params().depth()) {
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
			par = TeXDeeper(buf, paragraphs, par, os, texrow,
					runparams);
		}
	} while (par != const_cast<ParagraphList&>(paragraphs).end()
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


InsetOptArg * optArgInset(Paragraph const & par)
{
	// Find the entry.
	InsetList::const_iterator it = par.insetlist.begin();
	InsetList::const_iterator end = par.insetlist.end();
	for (; it != end; ++it) {
		InsetOld * ins = it->inset;
		if (ins->lyxCode() == InsetOld::OPTARG_CODE) {
			return static_cast<InsetOptArg *>(ins);
		}
	}
	return 0;
}


ParagraphList::iterator
TeXOnePar(Buffer const * buf,
	  ParagraphList const & paragraphs,
	  ParagraphList::iterator pit,
	  ostream & os, TexRow & texrow,
	  LatexRunParams const & runparams,
	  string const & everypar)
{
	lyxerr[Debug::LATEX] << "TeXOnePar...     " << &*pit << " '" << everypar
<< "'" << endl;
	BufferParams const & bparams = buf->params;

	InsetOld const * in = pit->inInset();
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
			&& (pit == const_cast<ParagraphList&>(paragraphs).begin() || !boost::prior(pit)->hasSameLayout(*pit))) {
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
			os << "\\lyxline{\\"
			   << pit->getFont(bparams, 0, outerFont(pit, paragraphs)).latexSize()
			   << '}'
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
	Language const * previous_language =
		(pit != const_cast<ParagraphList&>(paragraphs).begin())
		? boost::prior(pit)->getParLanguage(bparams)
		: doc_language;

	if (language->babel() != previous_language->babel()
	    // check if we already put language command in TeXEnvironment()
	    && !(style->isEnvironment()
		 && (pit == const_cast<ParagraphList&>(paragraphs).begin() ||
		     (boost::prior(pit)->layout() != pit->layout() &&
		      boost::prior(pit)->getDepth() <= pit->getDepth())
		     || boost::prior(pit)->getDepth() < pit->getDepth())))
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
				it->latexOptional(buf, os, runparams);
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

	os << everypar;
	bool need_par = pit->simpleTeXOnePar(buf, bparams,
					     outerFont(pit, paragraphs),
					     os, texrow, runparams);

	// Make sure that \\par is done with the font of the last
	// character if this has another size as the default.
	// This is necessary because LaTeX (and LyX on the screen)
	// calculates the space between the baselines according
	// to this font. (Matthias)
	//
	// Is this really needed ? (Dekel)
	// We do not need to use to change the font for the last paragraph
	// or for a command.
	LyXFont const outerfont(outerFont(pit, paragraphs));

	LyXFont const font =
		(pit->empty()
		 ? pit->getLayoutFont(bparams, outerfont)
		 : pit->getFont(bparams, pit->size() - 1, outerfont));

	bool is_command = style->isCommand();

	if (style->resfont.size() != font.size()
	    && boost::next(pit) != const_cast<ParagraphList&>(paragraphs).end()
	    && !is_command) {
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
		if (boost::next(pit) != const_cast<ParagraphList&>(paragraphs).end()
		    && (pit->params().depth() < boost::next(pit)->params().depth())) {
			os << '\n';
			texrow.newline();
		}
		break;
	case LATEX_ENVIRONMENT: {
		// if its the last paragraph of the current environment
		// skip it otherwise fall through
		ParagraphList::iterator next = boost::next(pit);

		if (next != const_cast<ParagraphList&>(paragraphs).end()
		    && (next->layout() != pit->layout()
			|| next->params().depth() != pit->params().depth()))
			break;
	}

		// fall through possible
	default:
		// we don't need it for the last paragraph!!!
		if (boost::next(pit) != const_cast<ParagraphList&>(paragraphs).end()) {
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
			&& (boost::next(pit) == const_cast<ParagraphList&>(paragraphs).end()|| !boost::next(pit)->hasSameLayout(*pit))) {
			os << pit->params().spacing().writeEnvirEnd() << '\n';
			texrow.newline();
		}
	}

	// we don't need it for the last paragraph!!!
	if (boost::next(pit) != const_cast<ParagraphList&>(paragraphs).end()) {
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

	lyxerr[Debug::LATEX] << "TeXOnePar...done " << &*boost::next(pit) << endl;
	return ++pit;
}

} // anon namespace


//
// LaTeX all paragraphs from par to endpar, if endpar == 0 then to the end
//
void latexParagraphs(Buffer const * buf,
		     ParagraphList const & paragraphs,
		     ostream & os,
		     TexRow & texrow,
		     LatexRunParams const & runparams,
		     string const & everypar)
{
	bool was_title = false;
	bool already_title = false;
	LyXTextClass const & tclass = buf->params.getLyXTextClass();
	ParagraphList::iterator par = const_cast<ParagraphList&>(paragraphs).begin();
	ParagraphList::iterator endpar = const_cast<ParagraphList&>(paragraphs).end();

	// if only_body
	while (par != endpar) {
		InsetOld * in = par->inInset();
		// well we have to check if we are in an inset with unlimited
		// length (all in one row) if that is true then we don't allow
		// any special options in the paragraph and also we don't allow
		// any environment other then "Standard" to be valid!
		if ((in == 0) || !in->forceDefaultParagraphs(in)) {
			LyXLayout_ptr const & layout = par->layout();

			if (layout->intitle) {
				if (already_title) {
					lyxerr <<"Error in latexParagraphs: You"
						" should not mix title layouts"
						" with normal ones." << endl;
				} else if (!was_title) {
					was_title = true;
					if (tclass.titletype() == TITLE_ENVIRONMENT) {
						os << "\\begin{"
						    << tclass.titlename()
						    << "}\n";
						texrow.newline();
					}
				}
			} else if (was_title && !already_title) {
				if (tclass.titletype() == TITLE_ENVIRONMENT) {
					os << "\\end{" << tclass.titlename()
					    << "}\n";
				}
				else {
					os << "\\" << tclass.titlename()
					    << "\n";
				}
				texrow.newline();
				already_title = true;
				was_title = false;
			}

			if (layout->is_environment) {
				par = TeXOnePar(buf, paragraphs, par, os, texrow,
				                runparams, everypar);
			} else if (layout->isEnvironment() ||
				!par->params().leftIndent().zero())
			{
				par = TeXEnvironment(buf, paragraphs, par, os,
						     texrow, runparams);
			} else {
				par = TeXOnePar(buf, paragraphs, par, os, texrow,
				                runparams, everypar);
			}
		} else {
			par = TeXOnePar(buf, paragraphs, par, os, texrow,
			                runparams, everypar);
		}
	}
	// It might be that we only have a title in this document
	if (was_title && !already_title) {
		if (tclass.titletype() == TITLE_ENVIRONMENT) {
			os << "\\end{" << tclass.titlename()
			    << "}\n";
		}
		else {
			os << "\\" << tclass.titlename()
			    << "\n";
				}
		texrow.newline();
	}
}


namespace {

int readParToken(Buffer & buf, Paragraph & par, LyXLex & lex, string const & token)
{
	static LyXFont font;
	static Change change;

	BufferParams const & bp = buf.params;

	if (token[0] != '\\') {
		string::const_iterator cit = token.begin();
		for (; cit != token.end(); ++cit) {
			par.insertChar(par.size(), (*cit), font, change);
		}
	} else if (token == "\\begin_layout") {
		lex.eatLine();
		string layoutname = lex.getString();

		font = LyXFont(LyXFont::ALL_INHERIT, bp.language);
		change = Change();

		LyXTextClass const & tclass = bp.getLyXTextClass();

		if (layoutname.empty()) {
			layoutname = tclass.defaultLayoutName();
		}

		bool hasLayout = tclass.hasLayout(layoutname);

		if (!hasLayout) {
			lyxerr << "Layout '" << layoutname << "' does not"
			       << " exist in textclass '" << tclass.name()
			       << "'." << endl;
			lyxerr << "Trying to use default layout instead."
			       << endl;
			layoutname = tclass.defaultLayoutName();
		}

		par.layout(bp.getLyXTextClass()[layoutname]);

		// Test whether the layout is obsolete.
		LyXLayout_ptr const & layout = par.layout();
		if (!layout->obsoleted_by().empty())
			par.layout(bp.getLyXTextClass()[layout->obsoleted_by()]);

		par.params().read(lex);

	} else if (token == "\\end_layout") {
		lyxerr << "Solitary \\end_layout in line " << lex.getLineNo() << "\n"
		       << "Missing \\begin_layout?.\n";
	} else if (token == "\\end_inset") {
		lyxerr << "Solitary \\end_inset in line " << lex.getLineNo() << "\n"
		       << "Missing \\begin_inset?.\n";
	} else if (token == "\\begin_inset") {
		InsetOld * inset = readInset(lex, buf);
		par.insertInset(par.size(), inset, font, change);
	} else if (token == "\\family") {
		lex.next();
		font.setLyXFamily(lex.getString());
	} else if (token == "\\series") {
		lex.next();
		font.setLyXSeries(lex.getString());
	} else if (token == "\\shape") {
		lex.next();
		font.setLyXShape(lex.getString());
	} else if (token == "\\size") {
		lex.next();
		font.setLyXSize(lex.getString());
	} else if (token == "\\lang") {
		lex.next();
		string const tok = lex.getString();
		Language const * lang = languages.getLanguage(tok);
		if (lang) {
			font.setLanguage(lang);
		} else {
			font.setLanguage(bp.language);
			lex.printError("Unknown language `$$Token'");
		}
	} else if (token == "\\numeric") {
		lex.next();
		font.setNumber(font.setLyXMisc(lex.getString()));
	} else if (token == "\\emph") {
		lex.next();
		font.setEmph(font.setLyXMisc(lex.getString()));
	} else if (token == "\\bar") {
		lex.next();
		string const tok = lex.getString();

		if (tok == "under")
			font.setUnderbar(LyXFont::ON);
		else if (tok == "no")
			font.setUnderbar(LyXFont::OFF);
		else if (tok == "default")
			font.setUnderbar(LyXFont::INHERIT);
		else
			lex.printError("Unknown bar font flag "
				       "`$$Token'");
	} else if (token == "\\noun") {
		lex.next();
		font.setNoun(font.setLyXMisc(lex.getString()));
	} else if (token == "\\color") {
		lex.next();
		font.setLyXColor(lex.getString());
	} else if (token == "\\InsetSpace" || token == "\\SpecialChar") {

		// Insets don't make sense in a free-spacing context! ---Kayvan
		if (par.isFreeSpacing()) {
			if (token == "\\InsetSpace")
				par.insertChar(par.size(), ' ', font, change);
			else if (lex.isOK()) {
				lex.next();
				string const next_token = lex.getString();
				if (next_token == "\\-")
					par.insertChar(par.size(), '-', font, change);
				else {
					lex.printError("Token `$$Token' "
						       "is in free space "
						       "paragraph layout!");
				}
			}
		} else {
			InsetOld * inset = 0;
			if (token == "\\SpecialChar" )
				inset = new InsetSpecialChar;
			else
				inset = new InsetSpace;
			inset->read(&buf, lex);
			par.insertInset(par.size(), inset, font, change);
		}
	} else if (token == "\\i") {
		InsetOld * inset = new InsetLatexAccent;
		inset->read(&buf, lex);
		par.insertInset(par.size(), inset, font, change);
	} else if (token == "\\backslash") {
		par.insertChar(par.size(), '\\', font, change);
	} else if (token == "\\newline") {
		InsetOld * inset = new InsetNewline;
		inset->read(&buf, lex);
		par.insertInset(par.size(), inset, font, change);
	} else if (token == "\\LyXTable") {
		InsetOld * inset = new InsetTabular(buf);
		inset->read(&buf, lex);
		par.insertInset(par.size(), inset, font, change);
	} else if (token == "\\bibitem") {
		InsetCommandParams p("bibitem", "dummy");
		InsetBibitem * inset = new InsetBibitem(p);
		inset->read(&buf, lex);
		par.insertInset(par.size(), inset, font, change);
	} else if (token == "\\hfill") {
		par.insertInset(par.size(), new InsetHFill(), font, change);
	} else if (token == "\\change_unchanged") {
		// Hack ! Needed for empty paragraphs :/
		// FIXME: is it still ??
		if (!par.size())
			par.cleanChanges();
		change = Change(Change::UNCHANGED);
	} else if (token == "\\change_inserted") {
		lex.nextToken();
		istringstream is(STRCONV(lex.getString()));
		int aid;
		lyx::time_type ct;
		is >> aid >> ct;
		change = Change(Change::INSERTED, bp.author_map[aid], ct);
	} else if (token == "\\change_deleted") {
		lex.nextToken();
		istringstream is(STRCONV(lex.getString()));
		int aid;
		lyx::time_type ct;
		is >> aid >> ct;
		change = Change(Change::DELETED, bp.author_map[aid], ct);
	} else {
		lex.eatLine();
		string const s = bformat(_("Unknown token: %1$s %2$s\n"),
			token, lex.getString());

		buf.error(ErrorItem(_("Unknown token"), s,
				    par.id(), 0, par.size()));
		return 1;
	}
	return 0;
}

}


int readParagraph(Buffer & buf, Paragraph & par, LyXLex & lex)
{
	int unknown = 0;

	lex.nextToken();
	string token = lex.getString();

	while (lex.isOK()) {

		unknown += readParToken(buf, par, lex, token);

		lex.nextToken();
		token = lex.getString();

		if (token.empty())
			continue;

		if (token == "\\end_layout") {
			//Ok, paragraph finished
			break;
		}

		lyxerr[Debug::PARSER] << "Handling paragraph token: `"
				      << token << '\'' << endl;
		if (token == "\\begin_layout" || token == "\\end_document"
		    || token == "\\end_inset" || token == "\\begin_deeper"
		    || token == "\\end_deeper") {
			lex.pushToken(token);
			lyxerr << "Paragraph ended in line "
			       << lex.getLineNo() << "\n"
			       << "Missing \\end_layout.\n";
			break;
		}
	}

	return unknown;
}


LyXFont const outerFont(ParagraphList::iterator pit,
			ParagraphList const & plist)
{
	Paragraph::depth_type par_depth = pit->getDepth();
	LyXFont tmpfont(LyXFont::ALL_INHERIT);

	// Resolve against environment font information
	while (pit != const_cast<ParagraphList&>(plist).end() &&
	       par_depth && !tmpfont.resolved()) {
		pit = outerHook(pit, plist);
		if (pit != const_cast<ParagraphList&>(plist).end()) {
			tmpfont.realize(pit->layout()->font);
			par_depth = pit->getDepth();
		}
	}

	return tmpfont;
}
