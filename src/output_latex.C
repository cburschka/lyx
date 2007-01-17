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

#include "insets/insetbibitem.h"
#include "insets/insetoptarg.h"

#include "support/lstrings.h"


namespace lyx {

using support::subst;

using std::endl;
using std::string;


namespace {

ParagraphList::const_iterator
TeXEnvironment(Buffer const & buf,
	       ParagraphList const & paragraphs,
	       ParagraphList::const_iterator pit,
	       odocstream & os, TexRow & texrow,
	       OutputParams const & runparams);

ParagraphList::const_iterator
TeXOnePar(Buffer const & buf,
	  ParagraphList const & paragraphs,
	  ParagraphList::const_iterator pit,
	  odocstream & os, TexRow & texrow,
	  OutputParams const & runparams,
	  string const & everypar = string());


ParagraphList::const_iterator
TeXDeeper(Buffer const & buf,
	  ParagraphList const & paragraphs,
	  ParagraphList::const_iterator pit,
	  odocstream & os, TexRow & texrow,
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
	lyxerr[Debug::LATEX] << "TeXDeeper...done " << endl;

	return par;
}


int latexOptArgInsets(Buffer const & buf, Paragraph const & par,
		      odocstream & os, OutputParams const & runparams, int number);


ParagraphList::const_iterator
TeXEnvironment(Buffer const & buf,
	       ParagraphList const & paragraphs,
	       ParagraphList::const_iterator pit,
	       odocstream & os, TexRow & texrow,
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
			os << from_ascii(subst(
				lyxrc.language_command_end,
				"$$lang",
				previous_language->babel()))
			   << '\n';
			texrow.newline();
		}

		if (lyxrc.language_command_end.empty() ||
		    language->babel() != doc_language->babel()) {
			os << from_ascii(subst(
				lyxrc.language_command_begin,
				"$$lang",
				language->babel()))
			   << '\n';
			texrow.newline();
		}
	}

	bool leftindent_open = false;
	if (!pit->params().leftIndent().zero()) {
		os << "\\begin{LyXParagraphLeftIndent}{"
		   << from_ascii(pit->params().leftIndent().asLatexString())
		   << "}\n";
		texrow.newline();
		leftindent_open = true;
	}

	if (style->isEnvironment()) {
		os << "\\begin{" << from_ascii(style->latexname()) << '}';
		if (style->optionalargs > 0) {
			int ret = latexOptArgInsets(buf, *pit, os, runparams,
						    style->optionalargs);
			while (ret > 0) {
				texrow.newline();
				--ret;
			}
		}
		if (style->latextype == LATEX_LIST_ENVIRONMENT) {
			os << '{'
			   << pit->params().labelWidthString()
			   << "}\n";
		} else if (style->labeltype == LABEL_BIBLIO) {
			// ale970405
			os << '{' << bibitemWidest(buf) << "}\n";
		} else
			os << from_ascii(style->latexparam()) << '\n';
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
		os << "\\end{" << from_ascii(style->latexname()) << "}\n";
		texrow.newline();
	}

	if (leftindent_open) {
		os << "\\end{LyXParagraphLeftIndent}\n";
		texrow.newline();
	}

	if (par != paragraphs.end() && lyxerr.debugging(Debug::LATEX))
		lyxerr << "TeXEnvironment...done " << &*par << endl;
	return par;
}


int latexOptArgInsets(Buffer const & buf, Paragraph const & par,
		      odocstream & os, OutputParams const & runparams, int number)
{
	int lines = 0;

	InsetList::const_iterator it = par.insetlist.begin();
	InsetList::const_iterator end = par.insetlist.end();
	for (; it != end && number > 0 ; ++it) {
		if (it->inset->lyxCode() == InsetBase::OPTARG_CODE) {
			InsetOptArg * ins =
				static_cast<InsetOptArg *>(it->inset);
			lines += ins->latexOptional(buf, os, runparams);
			--number;
		}
	}
	return lines;
}


ParagraphList::const_iterator
TeXOnePar(Buffer const & buf,
	  ParagraphList const & paragraphs,
	  ParagraphList::const_iterator pit,
	  odocstream & os, TexRow & texrow,
	  OutputParams const & runparams_in,
	  string const & everypar)
{
	lyxerr[Debug::LATEX] << "TeXOnePar...     " << &*pit << " '"
		<< everypar << "'" << endl;
	BufferParams const & bparams = buf.params();
	LyXLayout_ptr style;

	// In an inset with unlimited length (all in one row),
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
			os << from_ascii(subst(lyxrc.language_command_end,
				"$$lang",
				previous_language->babel()))
			   << '\n';
			texrow.newline();
		}

		if (lyxrc.language_command_end.empty() ||
		    language->babel() != doc_language->babel())
		{
			os << from_ascii(subst(
				lyxrc.language_command_begin,
				"$$lang",
				language->babel()))
			   << '\n';
			texrow.newline();
		}
	}

	LyXFont const outerfont =
		outerFont(std::distance(paragraphs.begin(), pit),
			  paragraphs);
	// This must be identical to basefont in Paragraph::simpleTeXOnePar
	LyXFont basefont = (pit->beginOfBody() > 0) ?
			pit->getLabelFont(bparams, outerfont) :
			pit->getLayoutFont(bparams, outerfont);
	Encoding const & outer_encoding(*(outerfont.language()->encoding()));
	// FIXME we switch from the outer encoding to the encoding of
	// this paragraph, since I could not figure out the correct
	// logic to take the encoding of the previous paragraph into
	// account. This may result in some unneeded encoding changes.
	if (switchEncoding(os, bparams, outer_encoding,
	                   *(basefont.language()->encoding()))) {
		os << '\n';
		texrow.newline();
	}

	// In an inset with unlimited length (all in one row),
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
			os << from_ascii(pit->params().spacing().writeEnvirBegin())
			    << '\n';
			texrow.newline();
		}

		if (style->isCommand()) {
			os << '\n';
			texrow.newline();
		}
	}

	switch (style->latextype) {
	case LATEX_COMMAND:
		os << '\\' << from_ascii(style->latexname());

		// Separate handling of optional argument inset.
		if (style->optionalargs > 0) {
			int ret = latexOptArgInsets(buf, *pit, os, runparams,
						    style->optionalargs);
			while (ret > 0) {
				texrow.newline();
				--ret;
			}
		}
		else
			os << from_ascii(style->latexparam());
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

	// FIXME UNICODE
	os << from_utf8(everypar);
	bool need_par = pit->simpleTeXOnePar(buf, bparams, outerfont,
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
		os << "\\" << from_ascii(font.latexSize()) << " \\par}";
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
		if (!pit->params().spacing().isDefault()
			&& (boost::next(pit) == paragraphs.end()
			    || !boost::next(pit)->hasSameLayout(*pit)))
		{
			os << from_ascii(pit->params().spacing().writeEnvirEnd())
			   << '\n';
			texrow.newline();
		}
	}

	if (boost::next(pit) == paragraphs.end()
	    && language->babel() != doc_language->babel()) {
		// Since \selectlanguage write the language to the aux file,
		// we need to reset the language at the end of footnote or
		// float.

		if (lyxrc.language_command_end.empty())
			os << from_ascii(subst(
				lyxrc.language_command_begin,
				"$$lang",
				doc_language->babel()))
			   << '\n';
		else
			os << from_ascii(subst(
				lyxrc.language_command_end,
				"$$lang",
				language->babel()))
			   << '\n';
		texrow.newline();
	}

	// FIXME we switch from the encoding of this paragraph to the
	// outer encoding, since I could not figure out the correct logic
	// to take the encoding of the next paragraph into account.
	// This may result in some unneeded encoding changes.
	basefont = pit->getLayoutFont(bparams, outerfont);
	if (switchEncoding(os, bparams, *(basefont.language()->encoding()),
	                   outer_encoding)) {
		os << '\n';
		texrow.newline();
	}

	// we don't need it for the last paragraph!!!
	// Note from JMarc: we will re-add a \n explicitely in
	// TeXEnvironment, because it is needed in this case
	if (boost::next(pit) != paragraphs.end()) {
		os << '\n';
		texrow.newline();
	}

	if (boost::next(pit) != paragraphs.end() &&
	    lyxerr.debugging(Debug::LATEX))
		lyxerr << "TeXOnePar...done " << &*boost::next(pit) << endl;

	return ++pit;
}

} // anon namespace


// LaTeX all paragraphs
void latexParagraphs(Buffer const & buf,
		     ParagraphList const & paragraphs,
		     odocstream & os,
		     TexRow & texrow,
		     OutputParams const & runparams,
		     string const & everypar)
{
	bool was_title = false;
	bool already_title = false;
	LyXTextClass const & tclass = buf.params().getLyXTextClass();
	ParagraphList::const_iterator par = paragraphs.begin();
	ParagraphList::const_iterator endpar = paragraphs.end();

	BOOST_ASSERT(runparams.par_begin <= runparams.par_end);
	// if only part of the paragraphs will be outputed
	if (runparams.par_begin !=  runparams.par_end) {
		par = boost::next(paragraphs.begin(), runparams.par_begin);
		endpar = boost::next(paragraphs.begin(), runparams.par_end);
		// runparams will be passed to nested paragraphs, so
		// we have to reset the range parameters.
		const_cast<OutputParams&>(runparams).par_begin = 0;
		const_cast<OutputParams&>(runparams).par_end = 0;
	}

	// if only_body
	while (par != endpar) {
		ParagraphList::const_iterator lastpar = par;
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
						    << from_ascii(tclass.titlename())
						    << "}\n";
						texrow.newline();
					}
				}
			} else if (was_title && !already_title) {
				if (tclass.titletype() == TITLE_ENVIRONMENT) {
					os << "\\end{" << from_ascii(tclass.titlename())
					    << "}\n";
				}
				else {
					os << "\\" << from_ascii(tclass.titlename())
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
		if (std::distance(lastpar, par) >= std::distance(lastpar, endpar))
			break;
	}
	// It might be that we only have a title in this document
	if (was_title && !already_title) {
		if (tclass.titletype() == TITLE_ENVIRONMENT) {
			os << "\\end{" << from_ascii(tclass.titlename())
			    << "}\n";
		}
		else {
			os << "\\" << from_ascii(tclass.titlename())
			    << "\n";
				}
		texrow.newline();
	}
}


int switchEncoding(odocstream & os, BufferParams const & bparams,
                   Encoding const & oldEnc, Encoding const & newEnc)
{
	// FIXME thailatex does not support the inputenc package, so we
	// ignore switches from/to tis620-0 encoding here. This does of
	// course only work as long as the non-thai text contains ASCII
	// only, but it is the best we can do.
	if ((bparams.inputenc == "auto" || bparams.inputenc == "default") &&
	    oldEnc.name() != newEnc.name() &&
	    oldEnc.name() != "tis620-0" && newEnc.name() != "tis620-0") {
		lyxerr[Debug::LATEX] << "Changing LaTeX encoding from "
		                     << oldEnc.name() << " to "
		                     << newEnc.name() << endl;
		os << setEncoding(newEnc.iconvName());
		if (bparams.inputenc != "default") {
			docstring const inputenc(from_ascii(newEnc.latexName()));
			os << "\\inputencoding{" << inputenc << '}';
			return 16 + inputenc.length();
		}
	}
	return 0;
}

} // namespace lyx
