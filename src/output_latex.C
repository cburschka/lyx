/**
 * \file output_latex.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "output_latex.h"

#include "buffer.h"
#include "bufferparams.h"
#include "debug.h"
#include "encoding.h"
#include "language.h"
#include "lyxrc.h"
#include "outputparams.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "texrow.h"
#include "vspace.h"

#include "insets/insetoptarg.h"

#include "support/lstrings.h"

#ifdef HAVE_LOCALE
#endif

using lyx::support::subst;

using std::endl;
using std::ostream;
using std::string;

extern string bibitemWidest(Buffer const &);


namespace {

ParagraphList::const_iterator
TeXEnvironment(Buffer const & buf,
	       ParagraphList const & paragraphs,
	       ParagraphList::const_iterator pit,
	       ostream & os, TexRow & texrow,
	       OutputParams const & runparams);

ParagraphList::const_iterator
TeXOnePar(Buffer const & buf,
	  ParagraphList const & paragraphs,
	  ParagraphList::const_iterator pit,
	  ostream & os, TexRow & texrow,
	  OutputParams const & runparams,
	  string const & everypar = string());


ParagraphList::const_iterator
TeXDeeper(Buffer const & buf,
	  ParagraphList const & paragraphs,
	  ParagraphList::const_iterator pit,
	  ostream & os, TexRow & texrow,
	  OutputParams const & runparams)
{
	lyxerr[Debug::LATEX] << "TeXDeeper...     " << &*pit << endl;
	ParagraphList::const_iterator par = pit;

	while (par != paragraphs.end() &&
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


ParagraphList::const_iterator
TeXEnvironment(Buffer const & buf,
	       ParagraphList const & paragraphs,
	       ParagraphList::const_iterator pit,
	       ostream & os, TexRow & texrow,
	       OutputParams const & runparams)
{
	lyxerr[Debug::LATEX] << "TeXEnvironment...     " << &*pit << endl;

	BufferParams const & bparams = buf.params();

	LyXLayout_ptr const & style = pit->layout();

	Language const * language = pit->getParLanguage(bparams);
	Language const * doc_language = bparams.language;
	Language const * previous_language =
		(pit != paragraphs.begin())
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
		os << "\\begin{" << style->latexname() << '}';
		if (style->latextype == LATEX_LIST_ENVIRONMENT) {
			os << "{" << pit->params().labelWidthString() << "}\n";
		} else if (style->labeltype == LABEL_BIBLIO) {
			// ale970405
			os << "{" <<  bibitemWidest(buf) << "}\n";
		} else
			os << style->latexparam() << '\n';
		texrow.newline();
	}
	ParagraphList::const_iterator par = pit;
	do {
		par = TeXOnePar(buf, paragraphs, par, os, texrow, runparams);

		if (par == paragraphs.end()) {
			// Make sure that the last paragraph is
			// correctly terminated (because TeXOnePar does
			// not add a \n in this case)
			os << '\n';
			texrow.newline();
		} else if (par->params().depth() > pit->params().depth()) {
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
	} while (par != paragraphs.end()
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
		InsetBase * ins = it->inset;
		if (ins->lyxCode() == InsetBase::OPTARG_CODE) {
			return static_cast<InsetOptArg *>(ins);
		}
	}
	return 0;
}


ParagraphList::const_iterator
TeXOnePar(Buffer const & buf,
	  ParagraphList const & paragraphs,
	  ParagraphList::const_iterator pit,
	  ostream & os, TexRow & texrow,
	  OutputParams const & runparams_in,
	  string const & everypar)
{
	lyxerr[Debug::LATEX] << "TeXOnePar...     " << &*pit << " '"
		<< everypar << "'" << endl;
	BufferParams const & bparams = buf.params();
	bool further_blank_line = false;
	LyXLayout_ptr style;

	// In an an inset with unlimited length (all in one row),
	// force layout to default
	if (!pit->forceDefaultParagraphs())
		style = pit->layout();
	else
		style = bparams.getLyXTextClass().defaultLayout();

	OutputParams runparams = runparams_in;
	runparams.moving_arg |= style->needprotect;

	Language const * language = pit->getParLanguage(bparams);
	Language const * doc_language = bparams.language;
	Language const * previous_language =
		(pit != paragraphs.begin())
		? boost::prior(pit)->getParLanguage(bparams)
		: doc_language;

	if (language->babel() != previous_language->babel()
	    // check if we already put language command in TeXEnvironment()
	    && !(style->isEnvironment()
		 && (pit == paragraphs.begin() ||
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

	// In an an inset with unlimited length (all in one row),
	// don't allow any special options in the paragraph
	if (!pit->forceDefaultParagraphs()) {
		if (pit->params().startOfAppendix()) {
			os << "\\appendix\n";
			texrow.newline();
		}

		if (!pit->params().spacing().isDefault()
			&& (pit == paragraphs.begin()
			    || !boost::prior(pit)->hasSameLayout(*pit)))
		{
			os << pit->params().spacing().writeEnvirBegin() << '\n';
			texrow.newline();
		}

		if (style->isCommand()) {
			os << '\n';
			texrow.newline();
		}

		if (further_blank_line) {
			os << '\n';
			texrow.newline();
		}
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
			outerFont(pit - paragraphs.begin(), paragraphs),
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
	LyXFont const outerfont =
			outerFont(pit - paragraphs.begin(),
paragraphs);

	LyXFont const font =
		(pit->empty()
		 ? pit->getLayoutFont(bparams, outerfont)
		 : pit->getFont(bparams, pit->size() - 1, outerfont));

	bool is_command = style->isCommand();

	if (style->resfont.size() != font.size()
	    && boost::next(pit) != paragraphs.end()
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
		if (boost::next(pit) != paragraphs.end()
		    && (pit->params().depth() < boost::next(pit)->params().depth())) {
			os << '\n';
			texrow.newline();
		}
		break;
	case LATEX_ENVIRONMENT: {
		// if its the last paragraph of the current environment
		// skip it otherwise fall through
		ParagraphList::const_iterator next = boost::next(pit);

		if (next != paragraphs.end()
		    && (next->layout() != pit->layout()
			|| next->params().depth() != pit->params().depth()))
			break;
	}

		// fall through possible
	default:
		// we don't need it for the last paragraph!!!
		if (boost::next(pit) != paragraphs.end()) {
			os << '\n';
			texrow.newline();
		}
	}

	if (!pit->forceDefaultParagraphs()) {
		further_blank_line = false;

		if (further_blank_line) {
			os << '\n';
			texrow.newline();
		}

		if (!pit->params().spacing().isDefault()
			&& (boost::next(pit) == paragraphs.end()
			    || !boost::next(pit)->hasSameLayout(*pit)))
		{
			os << pit->params().spacing().writeEnvirEnd() << '\n';
			texrow.newline();
		}
	}

	if (boost::next(pit) == const_cast<ParagraphList&>(paragraphs).end()
	    && language->babel() != doc_language->babel()) {
		// Since \selectlanguage write the language to the aux file,
		// we need to reset the language at the end of footnote or
		// float.

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

	// we don't need it for the last paragraph!!!
	// Note from JMarc: we will re-add a \n explicitely in
	// TeXEnvironment, because it is needed in this case
	if (boost::next(pit) != paragraphs.end()) {
		os << '\n';
		texrow.newline();
	}

	lyxerr[Debug::LATEX] << "TeXOnePar...done " << &*boost::next(pit) << endl;
	return ++pit;
}

} // anon namespace


// LaTeX all paragraphs
void latexParagraphs(Buffer const & buf,
		     ParagraphList const & paragraphs,
		     ostream & os,
		     TexRow & texrow,
		     OutputParams const & runparams,
		     string const & everypar)
{
	bool was_title = false;
	bool already_title = false;
	LyXTextClass const & tclass = buf.params().getLyXTextClass();
	ParagraphList::const_iterator par = paragraphs.begin();
	ParagraphList::const_iterator endpar = paragraphs.end();

	// if only_body
	while (par != endpar) {
		// well we have to check if we are in an inset with unlimited
		// length (all in one row) if that is true then we don't allow
		// any special options in the paragraph and also we don't allow
		// any environment other then "Standard" to be valid!
		if (!par->forceDefaultParagraphs()) {
			LyXLayout_ptr const & layout = par->layout();

			if (layout->intitle) {
				if (already_title) {
					lyxerr << "Error in latexParagraphs: You"
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
